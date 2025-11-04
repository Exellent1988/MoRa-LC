"""Race database models."""

from __future__ import annotations

from datetime import datetime
from enum import Enum as PyEnum

from sqlalchemy import Boolean, DateTime, Enum as SqlEnum, ForeignKey, Integer, String, func
from sqlalchemy.orm import Mapped, mapped_column, relationship

from app.db.base import Base


class RaceStatus(str, PyEnum):
    PLANNED = "planned"
    RUNNING = "running"
    PAUSED = "paused"
    FINISHED = "finished"


class Race(Base):
    __tablename__ = "races"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    name: Mapped[str] = mapped_column(String(120), nullable=False)
    duration_minutes: Mapped[int] = mapped_column(Integer, default=30)
    status: Mapped[RaceStatus] = mapped_column(
        SqlEnum(RaceStatus), default=RaceStatus.PLANNED, nullable=False
    )
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True), server_default=func.now(), nullable=False
    )
    started_at: Mapped[datetime | None] = mapped_column(DateTime(timezone=True), nullable=True)
    ended_at: Mapped[datetime | None] = mapped_column(DateTime(timezone=True), nullable=True)

    teams: Mapped[list["RaceTeam"]] = relationship(
        "RaceTeam", back_populates="race", cascade="all, delete-orphan"
    )

    def __repr__(self) -> str:  # pragma: no cover - debugging helper
        return f"<Race id={self.id} name={self.name!r} status={self.status}>"


class RaceTeam(Base):
    __tablename__ = "race_teams"

    race_id: Mapped[int] = mapped_column(ForeignKey("races.id", ondelete="CASCADE"), primary_key=True)
    team_id: Mapped[int] = mapped_column(ForeignKey("teams.id", ondelete="CASCADE"), primary_key=True)
    is_active: Mapped[bool] = mapped_column(Boolean, default=True, nullable=False)
    start_position: Mapped[int | None] = mapped_column(Integer, nullable=True)

    race: Mapped[Race] = relationship("Race", back_populates="teams")
    team: Mapped["Team"] = relationship("Team", back_populates="race_entries")


from app.models.team import Team  # noqa: E402  # Circular import resolution


__all__ = ["Race", "RaceTeam", "RaceStatus"]
