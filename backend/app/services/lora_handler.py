"""LoRa message handling service."""

from __future__ import annotations

import json
import logging
from dataclasses import dataclass
from typing import Any, Callable, Dict, List

from app.core.config import Settings
from app.utils import MQTTClient

logger = logging.getLogger(__name__)

LoRaCallback = Callable[["LoRaMessage"], None]


@dataclass
class LoRaMessage:
    """Representation of a decoded LoRa payload."""

    topic: str
    payload: Dict[str, Any]
    raw: bytes


class LoRaHandler:
    """Subscribe to LoRa MQTT messages and dispatch them to listeners."""

    def __init__(self, mqtt_client: MQTTClient, settings: Settings) -> None:
        self._mqtt = mqtt_client
        self._settings = settings
        self._listeners: List[LoRaCallback] = []
        self._running = False

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------
    def start(self) -> None:
        if self._running:
            return

        topic = self._settings.mqtt_lora_rx_topic
        self._mqtt.subscribe(topic, self._handle_message)
        self._running = True
        logger.info("LoRa handler listening on MQTT topic %s", topic)

    def stop(self) -> None:
        if not self._running:
            return

        topic = self._settings.mqtt_lora_rx_topic
        self._mqtt.unsubscribe(topic, self._handle_message)
        self._running = False
        logger.info("LoRa handler stopped listening on %s", topic)

    # ------------------------------------------------------------------
    # Listeners
    # ------------------------------------------------------------------
    def add_listener(self, callback: LoRaCallback) -> None:
        if callback not in self._listeners:
            self._listeners.append(callback)

    def remove_listener(self, callback: LoRaCallback) -> None:
        try:
            self._listeners.remove(callback)
        except ValueError:
            pass

    # ------------------------------------------------------------------
    # Internal MQTT callback
    # ------------------------------------------------------------------
    def _handle_message(self, topic: str, payload: bytes) -> None:
        try:
            decoded = self._decode_payload(payload)
        except ValueError:
            logger.warning("Discarding malformed LoRa payload from topic %s", topic)
            return

        message = LoRaMessage(topic=topic, payload=decoded, raw=payload)
        logger.debug("Received LoRa message: %s", message.payload)

        for listener in list(self._listeners):
            try:
                listener(message)
            except Exception:  # pragma: no cover - defensive dispatch
                logger.exception("LoRa listener raised an error")

    @staticmethod
    def _decode_payload(payload: bytes) -> Dict[str, Any]:
        try:
            text = payload.decode("utf-8")
        except UnicodeDecodeError as exc:
            raise ValueError("payload is not valid UTF-8") from exc

        try:
            data = json.loads(text)
        except json.JSONDecodeError as exc:
            raise ValueError("payload is not valid JSON") from exc

        if not isinstance(data, dict):
            raise ValueError("payload must decode to a JSON object")

        return data


__all__ = ["LoRaHandler", "LoRaMessage", "LoRaCallback"]
