from __future__ import annotations

from types import SimpleNamespace

import pytest

from app.utils.mqtt_client import MQTTClient


class DummyMQTTClient:
    def __init__(self, *args, **kwargs):  # pylint: disable=unused-argument
        self.on_connect = None
        self.on_disconnect = None
        self.on_message = None
        self.connected_args = None
        self.loop_started = False
        self.loop_stopped = False
        self.disconnected = False
        self.published: list[tuple[str, bytes, int, bool]] = []
        self.subscribed: list[tuple[str, int]] = []
        self.unsubscribed: list[str] = []

    # MQTT client API -------------------------------------------------
    def connect(self, host, port, keepalive):
        self.connected_args = (host, port, keepalive)

    def loop_start(self):
        self.loop_started = True

    def loop_stop(self):
        self.loop_stopped = True

    def disconnect(self):
        self.disconnected = True

    def publish(self, topic, payload, qos=0, retain=False):  # pylint: disable=unused-argument
        self.published.append((topic, payload, qos, retain))
        return SimpleNamespace(rc=0)

    def subscribe(self, topic, qos=0):
        self.subscribed.append((topic, qos))

    def unsubscribe(self, topic):
        self.unsubscribed.append(topic)

    # Helpers used by tests -------------------------------------------
    def trigger_connect(self, rc=0):
        if self.on_connect:
            self.on_connect(self, None, None, rc)

    def trigger_disconnect(self, rc=0):
        if self.on_disconnect:
            self.on_disconnect(self, None, rc)

    def trigger_message(self, topic: str, payload: bytes):
        if self.on_message:
            message = SimpleNamespace(topic=topic, payload=payload)
            self.on_message(self, None, message)


@pytest.fixture(autouse=True)
def patch_paho_client(monkeypatch):
    instances: list[DummyMQTTClient] = []

    def factory(*args, **kwargs):
        client = DummyMQTTClient(*args, **kwargs)
        instances.append(client)
        return client

    monkeypatch.setattr("app.utils.mqtt_client.mqtt.Client", factory)
    return instances


def make_settings(**overrides):
    defaults = {
        "app_name": "Test Backend",
        "mqtt_client_id": None,
        "mqtt_username": None,
        "mqtt_password": None,
        "mqtt_broker": "broker.local",
        "mqtt_port": 1883,
        "mqtt_keepalive": 30,
    }
    defaults.update(overrides)
    return SimpleNamespace(**defaults)


def test_mqtt_client_start_connects_and_subscribes(patch_paho_client):  # pylint: disable=unused-argument
    settings = make_settings()
    client = MQTTClient(settings)
    client.start()

    dummy = patch_paho_client[0]
    assert dummy.connected_args == (settings.mqtt_broker, settings.mqtt_port, settings.mqtt_keepalive)
    assert dummy.loop_started is True

    received: list[tuple[str, bytes]] = []

    def on_message(topic, payload):
        received.append((topic, payload))

    client.subscribe("mora/lora/rx", on_message)
    assert "mora/lora/rx" in client._subscriptions  # pylint: disable=protected-access
    assert dummy.subscribed == []

    dummy.trigger_connect()
    assert dummy.subscribed == [("mora/lora/rx", 0)]

    dummy.trigger_message("mora/lora/rx", b"payload")
    assert received == [("mora/lora/rx", b"payload")]

    client.stop()
    assert dummy.loop_stopped is True
    assert dummy.disconnected is True


def test_mqtt_client_publish_encodes_strings(patch_paho_client):  # pylint: disable=unused-argument
    settings = make_settings()
    client = MQTTClient(settings)
    client.start()

    dummy = patch_paho_client[0]
    dummy.trigger_connect()

    client.publish("topic/test", "hello", qos=1, retain=True)
    assert dummy.published == [("topic/test", b"hello", 1, True)]


def test_mqtt_client_unsubscribe_removes_callbacks(patch_paho_client):  # pylint: disable=unused-argument
    settings = make_settings()
    client = MQTTClient(settings)
    client.start()

    dummy = patch_paho_client[0]
    dummy.trigger_connect()

    received = []

    def callback(topic, payload):  # pragma: no cover - used indirectly
        received.append(payload)

    client.subscribe("topic/remove", callback)

    client.unsubscribe("topic/remove", callback)
    assert "topic/remove" not in client._subscriptions  # pylint: disable=protected-access
    assert dummy.unsubscribed == ["topic/remove"]

    dummy.trigger_message("topic/remove", b"ignored")
    assert received == []
