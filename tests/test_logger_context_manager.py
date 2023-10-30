"""Tests for nqm.irimager.Logging"""
import logging

import pytest

from nqm.irimager import LoggerContextManager


def test_logger_in_context_manager(caplog):
    """We should be able to wrap logger in a context manager"""

    with caplog.at_level(logging.DEBUG):
        with LoggerContextManager():
            assert "Redirecting spdlogs to a callback." in caplog.text

        caplog.clear()

        # creating a second logger should be fine after the first contextmanager
        # closes
        with LoggerContextManager():
            assert "Redirecting spdlogs to a callback." in caplog.text


def test_logger_in_context_manager_handles_exception(caplog):
    """We should be able to wrap logger in a context manager and handle exceptions"""

    with caplog.at_level(logging.DEBUG):
        # creating a second logger should be fine, even if the first
        # contextmanager broke due to an exception
        with pytest.raises(
            NotImplementedError,
            match="Testing whether contextmanager cleans up after errors",
        ):
            with LoggerContextManager():
                raise NotImplementedError(
                    "Testing whether contextmanager cleans up after errors"
                )

        caplog.clear()
        with LoggerContextManager():
            assert "Redirecting spdlogs to a callback." in caplog.text
