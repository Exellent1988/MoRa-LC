"""Pydantic schema definitions for teams."""

from __future__ import annotations

from datetime import datetime
from typing import Optional

from pydantic import BaseModel, Field, constr


TeamName = constr(strip_whitespace=True, min_length=1, max_length=80)


class TeamBase(BaseModel):
    name: TeamName = Field(..., description="Anzeigename des Teams")
    beacon_mac: Optional[str] = Field(
        None,
        description="Optionale BLE MAC-Adresse des zugeordneten Beacons",
        max_length=32,
    )


class TeamCreate(TeamBase):
    pass


class TeamUpdate(BaseModel):
    name: Optional[TeamName] = None
    beacon_mac: Optional[str] = Field(None, max_length=32)


class TeamBeaconAssign(BaseModel):
    beacon_mac: str = Field(..., max_length=32)


class TeamRead(TeamBase):
    id: int
    created_at: datetime

    class Config:
        orm_mode = True


class TeamSummary(BaseModel):
    id: int
    name: str

    class Config:
        orm_mode = True


__all__ = [
    "TeamBase",
    "TeamCreate",
    "TeamUpdate",
    "TeamBeaconAssign",
    "TeamRead",
    "TeamSummary",
]
