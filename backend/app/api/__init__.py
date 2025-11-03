"""API router aggregation."""

from fastapi import APIRouter

from . import races, teams

api_router = APIRouter()
api_router.include_router(teams.router, prefix="/api")
api_router.include_router(races.router, prefix="/api")

__all__ = ["api_router"]
