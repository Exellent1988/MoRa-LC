"""Race API endpoints."""

from __future__ import annotations

from datetime import datetime
from typing import List, Sequence

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy import select
from sqlalchemy.orm import Session, joinedload

from app.db.session import get_db
from app.models import Race, RaceStatus, RaceTeam, Team
from app.schemas import RaceCreate, RaceRead, RaceUpdate, TeamSummary

router = APIRouter(prefix="/races", tags=["races"])


def _get_race_or_404(race_id: int, db: Session) -> Race:
    race = (
        db.execute(select(Race).options(joinedload(Race.teams).joinedload(RaceTeam.team)).where(Race.id == race_id))
        .unique()
        .scalar_one_or_none()
    )
    if not race:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Rennen nicht gefunden")
    return race


def _serialize_race(race: Race) -> RaceRead:
    teams = [TeamSummary.from_orm(entry.team) for entry in race.teams]
    return RaceRead(
        id=race.id,
        name=race.name,
        duration_minutes=race.duration_minutes,
        status=race.status,
        created_at=race.created_at,
        started_at=race.started_at,
        ended_at=race.ended_at,
        teams=teams,
    )


def _fetch_teams(team_ids: Sequence[int], db: Session) -> List[Team]:
    if not team_ids:
        return []

    teams = db.execute(select(Team).where(Team.id.in_(team_ids))).scalars().all()
    missing = set(team_ids) - {team.id for team in teams}
    if missing:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Unbekannte Team-IDs: {sorted(missing)}",
        )
    return teams


@router.get("/", response_model=List[RaceRead])
def list_races(db: Session = Depends(get_db)) -> List[RaceRead]:
    races = (
        db.execute(select(Race).options(joinedload(Race.teams).joinedload(RaceTeam.team)).order_by(Race.created_at.desc()))
        .unique()
        .scalars()
        .all()
    )
    return [_serialize_race(race) for race in races]


@router.post("/", response_model=RaceRead, status_code=status.HTTP_201_CREATED)
def create_race(payload: RaceCreate, db: Session = Depends(get_db)) -> RaceRead:
    race = Race(name=payload.name, duration_minutes=payload.duration_minutes)

    teams = _fetch_teams(payload.team_ids or [], db)
    for team in teams:
        race.teams.append(RaceTeam(team=team))

    db.add(race)
    db.flush()
    db.refresh(race)

    return _serialize_race(race)


@router.get("/{race_id}", response_model=RaceRead)
def get_race(race_id: int, db: Session = Depends(get_db)) -> RaceRead:
    race = _get_race_or_404(race_id, db)
    return _serialize_race(race)


@router.put("/{race_id}", response_model=RaceRead)
def update_race(race_id: int, payload: RaceUpdate, db: Session = Depends(get_db)) -> RaceRead:
    race = _get_race_or_404(race_id, db)

    if payload.name is not None:
        race.name = payload.name
    if payload.duration_minutes is not None:
        race.duration_minutes = payload.duration_minutes
    if payload.status is not None:
        race.status = payload.status
    if payload.started_at is not None:
        race.started_at = payload.started_at
    if payload.ended_at is not None:
        race.ended_at = payload.ended_at

    db.add(race)
    db.flush()
    db.refresh(race)

    return _serialize_race(race)


@router.put("/{race_id}/start", response_model=RaceRead)
def start_race(race_id: int, db: Session = Depends(get_db)) -> RaceRead:
    race = _get_race_or_404(race_id, db)

    if race.status not in {RaceStatus.PLANNED, RaceStatus.PAUSED}:
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="Rennen kann nicht gestartet werden")

    race.status = RaceStatus.RUNNING
    if race.started_at is None:
        race.started_at = datetime.utcnow()

    db.add(race)
    db.flush()
    db.refresh(race)
    return _serialize_race(race)


@router.put("/{race_id}/pause", response_model=RaceRead)
def pause_race(race_id: int, db: Session = Depends(get_db)) -> RaceRead:
    race = _get_race_or_404(race_id, db)

    if race.status != RaceStatus.RUNNING:
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="Rennen läuft nicht")

    race.status = RaceStatus.PAUSED

    db.add(race)
    db.flush()
    db.refresh(race)
    return _serialize_race(race)


@router.put("/{race_id}/stop", response_model=RaceRead)
def stop_race(race_id: int, db: Session = Depends(get_db)) -> RaceRead:
    race = _get_race_or_404(race_id, db)

    if race.status not in {RaceStatus.RUNNING, RaceStatus.PAUSED}:
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="Rennen läuft nicht")

    race.status = RaceStatus.FINISHED
    race.ended_at = datetime.utcnow()

    db.add(race)
    db.flush()
    db.refresh(race)
    return _serialize_race(race)


__all__ = ["router"]
