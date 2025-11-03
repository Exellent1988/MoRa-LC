from __future__ import annotations

from app.services import LoRaHandler


class FakeMQTTClient:
    def __init__(self) -> None:
        self.subscriptions: dict[str, tuple] = {}
        self.unsubscriptions: list[str] = []

    def subscribe(self, topic: str, callback, qos: int = 0):  # pylint: disable=unused-argument
        self.subscriptions[topic] = (callback, qos)

    def unsubscribe(self, topic: str, callback=None):  # pylint: disable=unused-argument
        self.unsubscriptions.append(topic)
        self.subscriptions.pop(topic, None)


def make_settings(**overrides):
    defaults = {
        "mqtt_lora_rx_topic": "mora/lora/rx",
    }
    defaults.update(overrides)
    return SimpleNamespace(**defaults)


def test_lora_handler_dispatches_valid_json():
    mqtt_client = FakeMQTTClient()
    settings = make_settings()

    handler = LoRaHandler(mqtt_client, settings)
    handler.start()

    assert settings.mqtt_lora_rx_topic in mqtt_client.subscriptions

    captured = []

    def listener(message):
        captured.append(message.payload)

    handler.add_listener(listener)

    callback, _ = mqtt_client.subscriptions[settings.mqtt_lora_rx_topic]
    callback(settings.mqtt_lora_rx_topic, b"{\"speed\": 42}")

    assert captured == [{"speed": 42}]

    handler.stop()
    assert mqtt_client.unsubscriptions == [settings.mqtt_lora_rx_topic]


def test_lora_handler_ignores_invalid_payload():
    mqtt_client = FakeMQTTClient()
    settings = make_settings()

    handler = LoRaHandler(mqtt_client, settings)
    handler.start()

    captured = []

    def listener(message):
        captured.append(message.payload)

    handler.add_listener(listener)

    callback, _ = mqtt_client.subscriptions[settings.mqtt_lora_rx_topic]
    callback(settings.mqtt_lora_rx_topic, b"not-json")

    assert captured == []
