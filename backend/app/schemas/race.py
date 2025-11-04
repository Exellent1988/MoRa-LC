"""Pydantic schema definitions for races."""

from __future__ import annotations

from datetime import datetime
from typing import List, Optional

from pydantic import BaseModel, Field, conint, constr

from app.models.race import RaceStatus
from app.schemas.team import TeamSummary


RaceName = constr(strip_whitespace=True, min_length=1, max_length=120)


class RaceBase(BaseModel):
    name: RaceName = Field(..., description="Name des Rennens")
    duration_minutes: conint(ge=1, le=360) = Field(30, description="Dauer in Minuten")


class RaceCreate(RaceBase):
    team_ids: Optional[List[int]] = Field(
        default=None, description="Optionale Liste von Team-IDs, die teilnehmen sollen"
    )


class RaceUpdate(BaseModel):
    name: Optional[RaceName] = None
    duration_minutes: Optional[conint(ge=1, le=360)] = None
    status: Optional[RaceStatus] = None
    started_at: Optional[datetime] = None
    ended_at: Optional[datetime] = None


class RaceRead(RaceBase):
    id: int
    status: RaceStatus
    created_at: datetime
    started_at: Optional[datetime]
    ended_at: Optional[datetime]
    teams: List[TeamSummary] = Field(default_factory=list)

    class Config:
        orm_mode = True


__all__ = ["RaceBase", "RaceCreate", "RaceUpdate", "RaceRead"]
