"""FastAPI application entry point."""

from __future__ import annotations

from fastapi import FastAPI

from app.api import api_router
from app.core.config import get_settings
from app.db.session import init_db


def create_app() -> FastAPI:
    settings = get_settings()

    app = FastAPI(title=settings.app_name, debug=settings.debug)

    @app.on_event("startup")
    def _startup_event() -> None:
        init_db()

    @app.get("/health", tags=["meta"], summary="Health Check")
    def healthcheck() -> dict[str, str]:
        return {"status": "ok"}

    app.include_router(api_router)

    return app


app = create_app()


__all__ = ["create_app", "app"]
