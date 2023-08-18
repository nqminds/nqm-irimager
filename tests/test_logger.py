"""Tests for nqm.irimager.Logging"""
import logging

import pytest

from nqm.irimager import Logger


def test_logger_basic():
    """Test whether the Logger object runs and exits"""
    logger = Logger()
    del logger


def test_logger_should_capture_data(caplog):
    """Test whether the Logger object passes data to a Python logger"""

    # nothing should be logged at the INFO level
    with caplog.at_level(logging.INFO):
        Logger()

        assert "Redirecting spdlogs to a callback." not in caplog.text

    # something should be logged at the DEBUG level
    with caplog.at_level(logging.DEBUG):
        Logger()

        assert "Redirecting spdlogs to a callback." in caplog.text


def test_logger_should_ban_multiple_instances():
    """Should not allow multiple instances at once"""

    first_logger = Logger()

    with pytest.raises(
        RuntimeError,
        match="Only a single instance of the Logger should be active at the same time.",
    ):
        Logger()

    # creating a second logger should be fine, if the first one is deleted
    del first_logger
    Logger()
