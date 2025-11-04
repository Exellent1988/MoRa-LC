"""Minimal MQTT client wrapper used by backend services."""

from __future__ import annotations

import logging
import threading
from typing import Callable, Dict, List, Optional

import paho.mqtt.client as mqtt

from app.core.config import Settings

logger = logging.getLogger(__name__)

MessageCallback = Callable[[str, bytes], None]


class MQTTClient:
    """Wrapper around paho-mqtt with topic-based callback dispatching."""

    def __init__(self, settings: Settings) -> None:
        self._settings = settings
        client_id = settings.mqtt_client_id or settings.app_name.replace(" ", "-")
        self._client = mqtt.Client(client_id=client_id, clean_session=True)

        if settings.mqtt_username:
            self._client.username_pw_set(settings.mqtt_username, settings.mqtt_password or "")

        self._client.on_connect = self._on_connect
        self._client.on_message = self._on_message
        self._client.on_disconnect = self._on_disconnect

        self._subscriptions: Dict[str, List[MessageCallback]] = {}
        self._lock = threading.RLock()
        self._loop_started = False
        self._connected = False

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------
    def start(self) -> None:
        if self._loop_started:
            return

        try:
            logger.info(
                "Connecting to MQTT broker %s:%s", self._settings.mqtt_broker, self._settings.mqtt_port
            )
            self._client.connect(
                self._settings.mqtt_broker,
                self._settings.mqtt_port,
                keepalive=self._settings.mqtt_keepalive,
            )
            self._client.loop_start()
            self._loop_started = True
        except Exception as exc:  # pragma: no cover - runtime safeguard
            logger.exception("Failed to connect to MQTT broker: %s", exc)
            raise

    def stop(self) -> None:
        if not self._loop_started:
            return

        logger.info("Stopping MQTT client")
        self._client.loop_stop()
        self._client.disconnect()
        self._loop_started = False
        self._connected = False

    # ------------------------------------------------------------------
    # Publish & Subscribe
    # ------------------------------------------------------------------
    def publish(self, topic: str, payload: str | bytes, qos: int = 0, retain: bool = False) -> None:
        if isinstance(payload, str):
            payload = payload.encode("utf-8")
        result = self._client.publish(topic, payload=payload, qos=qos, retain=retain)
        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            logger.warning("Failed to publish MQTT message to %s (rc=%s)", topic, result.rc)

    def subscribe(self, topic: str, callback: MessageCallback, qos: int = 0) -> None:
        with self._lock:
            callbacks = self._subscriptions.setdefault(topic, [])
            if callback not in callbacks:
                callbacks.append(callback)
                logger.debug("Registered MQTT callback for topic %s", topic)

            if self._connected:
                self._client.subscribe(topic, qos=qos)

    def unsubscribe(self, topic: str, callback: Optional[MessageCallback] = None) -> None:
        with self._lock:
            if topic not in self._subscriptions:
                return

            if callback:
                try:
                    self._subscriptions[topic].remove(callback)
                    logger.debug("Removed MQTT callback for topic %s", topic)
                except ValueError:
                    pass
            else:
                self._subscriptions[topic].clear()

            if not self._subscriptions[topic]:
                self._subscriptions.pop(topic, None)
                if self._connected:
                    self._client.unsubscribe(topic)

    # ------------------------------------------------------------------
    # MQTT Event Handlers
    # ------------------------------------------------------------------
    def _on_connect(self, client: mqtt.Client, userdata, flags, rc):  # pylint: disable=unused-argument
        if rc == 0:
            logger.info("Connected to MQTT broker")
            self._connected = True
            with self._lock:
                for topic in self._subscriptions.keys():
                    client.subscribe(topic)
        else:
            logger.error("MQTT connection failed with status code %s", rc)

    def _on_disconnect(self, client: mqtt.Client, userdata, rc):  # pylint: disable=unused-argument
        self._connected = False
        if rc != 0:
            logger.warning("Unexpected MQTT disconnect (rc=%s)", rc)

    def _on_message(self, client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):  # pylint: disable=unused-argument
        with self._lock:
            callbacks = list(self._subscriptions.get(msg.topic, []))

        if not callbacks:
            logger.debug("No MQTT callbacks registered for topic %s", msg.topic)
            return

        for callback in callbacks:
            try:
                callback(msg.topic, msg.payload)
            except Exception:  # pragma: no cover - defensive dispatch
                logger.exception("Error while handling MQTT message on %s", msg.topic)


__all__ = ["MQTTClient", "MessageCallback"]
