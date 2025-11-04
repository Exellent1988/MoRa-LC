"""Team API endpoints."""

from __future__ import annotations

from typing import List

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy import select
from sqlalchemy.orm import Session

from app.db.session import get_db
from app.models.team import Team
from app.schemas.team import TeamBeaconAssign, TeamCreate, TeamRead, TeamSummary, TeamUpdate

router = APIRouter(prefix="/teams", tags=["teams"])


def _get_team_or_404(team_id: int, db: Session) -> Team:
    team = db.execute(select(Team).where(Team.id == team_id)).scalar_one_or_none()
    if not team:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Team nicht gefunden")
    return team


@router.get("/", response_model=List[TeamRead])
def list_teams(db: Session = Depends(get_db)) -> List[TeamRead]:
    teams = db.execute(select(Team).order_by(Team.created_at)).scalars().all()
    return teams


@router.post("/", response_model=TeamRead, status_code=status.HTTP_201_CREATED)
def create_team(payload: TeamCreate, db: Session = Depends(get_db)) -> TeamRead:
    existing = db.execute(select(Team).where(Team.name == payload.name)).scalar_one_or_none()
    if existing:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="Team mit diesem Namen existiert bereits",
        )

    team = Team(name=payload.name, beacon_mac=payload.beacon_mac)
    db.add(team)
    db.flush()  # Ensures ID is generated
    db.refresh(team)
    return team


@router.get("/{team_id}", response_model=TeamRead)
def get_team(team_id: int, db: Session = Depends(get_db)) -> TeamRead:
    team = _get_team_or_404(team_id, db)
    return team


@router.put("/{team_id}", response_model=TeamRead)
def update_team(team_id: int, payload: TeamUpdate, db: Session = Depends(get_db)) -> TeamRead:
    team = _get_team_or_404(team_id, db)

    if payload.name and payload.name != team.name:
        duplicate = db.execute(select(Team).where(Team.name == payload.name)).scalar_one_or_none()
        if duplicate and duplicate.id != team.id:
            raise HTTPException(
                status_code=status.HTTP_409_CONFLICT,
                detail="Team mit diesem Namen existiert bereits",
            )

    if payload.name is not None:
        team.name = payload.name
    if payload.beacon_mac is not None:
        team.beacon_mac = payload.beacon_mac

    db.add(team)
    db.flush()
    db.refresh(team)
    return team


@router.delete("/{team_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_team(team_id: int, db: Session = Depends(get_db)) -> None:
    team = _get_team_or_404(team_id, db)
    db.delete(team)
    db.flush()


@router.post("/{team_id}/beacon", response_model=TeamRead)
def assign_beacon(team_id: int, payload: TeamBeaconAssign, db: Session = Depends(get_db)) -> TeamRead:
    team = _get_team_or_404(team_id, db)
    team.beacon_mac = payload.beacon_mac
    db.add(team)
    db.flush()
    db.refresh(team)
    return team


@router.get("/summaries", response_model=List[TeamSummary])
def list_team_summaries(db: Session = Depends(get_db)) -> List[TeamSummary]:
    teams = db.execute(select(Team.id, Team.name).order_by(Team.created_at)).all()
    return [TeamSummary(id=row.id, name=row.name) for row in teams]


__all__ = ["router"]
