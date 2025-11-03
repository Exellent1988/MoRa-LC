"""Database engine and session management."""

from __future__ import annotations

from contextlib import contextmanager
from typing import Iterator

from sqlalchemy import create_engine
from sqlalchemy.orm import Session, sessionmaker

from app.core.config import get_settings
from app.db.base import Base


def _create_engine_url() -> str:
    return get_settings().database_url


def _create_engine():
    settings = get_settings()
    engine_kwargs = {"future": True}

    url = settings.database_url
    if url.startswith("sqlite"):
        engine_kwargs["connect_args"] = {"check_same_thread": False}

    engine_kwargs["echo"] = settings.debug
    return create_engine(url, **engine_kwargs)


engine = _create_engine()

SessionLocal = sessionmaker(bind=engine, autocommit=False, autoflush=False)


def init_db() -> None:
    """Create all tables."""

    # Import models for metadata registration
    import app.models  # noqa: F401  # pylint: disable=unused-import

    Base.metadata.create_all(bind=engine)


@contextmanager
def session_scope() -> Iterator[Session]:
    """Provide a transactional scope around a series of operations."""

    session = SessionLocal()
    try:
        yield session
        session.commit()
    except Exception:  # pragma: no cover - transactional safety
        session.rollback()
        raise
    finally:
        session.close()


def get_db() -> Iterator[Session]:
    """FastAPI dependency that yields a database session."""

    with session_scope() as session:
        yield session


__all__ = ["engine", "SessionLocal", "init_db", "get_db", "session_scope"]
