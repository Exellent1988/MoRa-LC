"""Application configuration management.

The backend expects a `config.yaml` file in the `backend/` directory. Values can
also be overridden through environment variables or an optional `.env` file.
"""

from __future__ import annotations

from functools import lru_cache
from pathlib import Path
from typing import Any, Dict

import yaml
from pydantic import BaseSettings, Field


class Settings(BaseSettings):
    """Global application settings."""

    app_name: str = Field("MoRa-LC Backend", env="APP_NAME")
    debug: bool = Field(False, env="DEBUG")

    database_url: str = Field("sqlite:///./data/mora.db", env="DATABASE_URL")

    mqtt_broker: str = Field("localhost", env="MQTT_BROKER")
    mqtt_port: int = Field(1883, env="MQTT_PORT")
    mqtt_lora_rx_topic: str = Field("mora/lora/rx", env="MQTT_TOPIC_LORA_RX")
    mqtt_lora_tx_topic: str = Field("mora/lora/tx", env="MQTT_TOPIC_LORA_TX")

    track_size_x: int = Field(75, env="TRACK_SIZE_X")
    track_size_y: int = Field(75, env="TRACK_SIZE_Y")

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"


def _load_yaml_config(payload: Dict[str, Any] | None, base: Settings) -> Settings:
    if not payload:
        return base

    updates: Dict[str, Any] = {}

    database = payload.get("database") or {}
    if isinstance(database, dict) and database.get("url"):
        updates["database_url"] = database["url"]

    mqtt = payload.get("mqtt") or {}
    if isinstance(mqtt, dict):
        if mqtt.get("broker"):
            updates["mqtt_broker"] = mqtt["broker"]
        if mqtt.get("port") is not None:
            updates["mqtt_port"] = int(mqtt["port"])
        topics = mqtt.get("topics") or {}
        if isinstance(topics, dict):
            if topics.get("lora_rx"):
                updates["mqtt_lora_rx_topic"] = topics["lora_rx"]
            if topics.get("lora_tx"):
                updates["mqtt_lora_tx_topic"] = topics["lora_tx"]

    track = payload.get("track") or {}
    if isinstance(track, dict):
        if track.get("size_x") is not None:
            updates["track_size_x"] = int(track["size_x"])
        if track.get("size_y") is not None:
            updates["track_size_y"] = int(track["size_y"])

    general = payload.get("general") or {}
    if isinstance(general, dict):
        if general.get("app_name"):
            updates["app_name"] = general["app_name"]
        if general.get("debug") is not None:
            updates["debug"] = bool(general["debug"])

    return base.copy(update=updates)


@lru_cache()
def get_settings() -> Settings:
    settings = Settings()

    yaml_path = Path(__file__).resolve().parents[2] / "config.yaml"
    if yaml_path.exists():
        try:
            with yaml_path.open("r", encoding="utf-8") as fh:
                data = yaml.safe_load(fh)
            settings = _load_yaml_config(data, settings)
        except Exception as exc:  # pragma: no cover - defensive logging
            raise RuntimeError(f"Failed to load configuration from {yaml_path}: {exc}") from exc

    return settings


__all__ = ["Settings", "get_settings"]
