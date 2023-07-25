"""Tests for nqm.irimager.IRImager"""
import datetime
import pathlib

import numpy as np
import pytest

from nqm.irimager import IRImager


def test_irimager_loads_xml():
    """Tests nqm.irimager.IRImager(xmlPath) can load an XML file"""
    # should work with a valid XML file
    IRImager(pathlib.Path(__file__).parent / "__fixtures__" / "382x288@27Hz.xml")

    with pytest.raises(RuntimeError, match="Invalid XML file"):
        IRImager(pathlib.Path("README.md"))


def test_get_frame_fails_when_not_streaming():
    """Calling `get_frame()` should raise an error when not streaming"""
    irimager = IRImager()

    with pytest.raises(RuntimeError, match="IRIMAGER_STREAMOFF"):
        irimager.get_frame()


def test_get_frame_in_context_manager():
    """Calling `get_frame()` should work when starting streaming with `with`"""
    irimager = IRImager()

    # context manager should auto-start streaming
    with irimager:
        irimager.get_frame()

    # context manager should auto-stop streaming
    with pytest.raises(RuntimeError, match="IRIMAGER_STREAMOFF"):
        irimager.get_frame()


def test_irimager_get_frame():
    """Tests nqm.irimager.IRImager#get_frame"""
    irimager = IRImager()

    with irimager:
        array, timestamp = irimager.get_frame()

        assert array.dtype == np.uint16
        # should be 2-dimensional
        assert array.ndim == 2
        assert array.shape == (128, 128)

        # image should have been taken in the last 30 seconds
        assert timestamp > datetime.datetime.now() - datetime.timedelta(seconds=30)


def test_irimager_get_temp_range_decimal():
    """Tests that nqm.irimager.IRImager#get_temp_range_decimal returns an int"""
    irimager = IRImager()

    assert irimager.get_temp_range_decimal() >= 0
    assert isinstance(irimager.get_temp_range_decimal(), int)
