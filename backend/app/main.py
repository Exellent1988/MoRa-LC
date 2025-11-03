"""FastAPI application entry point."""

from __future__ import annotations

import logging

from fastapi import FastAPI

from app.api import api_router
from app.core.config import get_settings
from app.db.session import init_db
from app.services import LoRaHandler
from app.utils import MQTTClient

logger = logging.getLogger(__name__)


def create_app() -> FastAPI:
    settings = get_settings()

    app = FastAPI(title=settings.app_name, debug=settings.debug)

    @app.on_event("startup")
    def _startup_event() -> None:
        init_db()

        try:
            mqtt_client = MQTTClient(settings)
            mqtt_client.start()
            app.state.mqtt_client = mqtt_client

            lora_handler = LoRaHandler(mqtt_client, settings)
            lora_handler.start()
            app.state.lora_handler = lora_handler
        except Exception as exc:  # pragma: no cover - runtime safeguard
            logger.exception("Unable to start MQTT/LoRa services: %s", exc)

    @app.get("/health", tags=["meta"], summary="Health Check")
    def healthcheck() -> dict[str, str]:
        return {"status": "ok"}

    @app.on_event("shutdown")
    def _shutdown_event() -> None:
        lora_handler = getattr(app.state, "lora_handler", None)
        if lora_handler:
            lora_handler.stop()

        mqtt_client = getattr(app.state, "mqtt_client", None)
        if mqtt_client:
            mqtt_client.stop()

    app.include_router(api_router)

    return app


app = create_app()


__all__ = ["create_app", "app"]
