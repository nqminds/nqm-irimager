"""Tests for nqm.irimager.IRImager"""
import datetime
import pathlib

import numpy as np
import pytest

from nqm.irimager import IRImagerMock as IRImager
from nqm.irimager import Logger

XML_FILE = pathlib.Path(__file__).parent / "__fixtures__" / "382x288@27Hz.xml"
README_FILE = pathlib.Path(__file__).parent.parent / "README.md"


@pytest.fixture(scope="module", autouse=True)
def logger():
    """Moves spdlog into Python's logging module, for better pytest compatibility"""
    my_logger = Logger()
    yield
    del my_logger


def test_irimager_loads_xml():
    """Tests nqm.irimager.IRImager(xmlPath) can load an XML file"""
    # should work with a valid XML file
    IRImager(XML_FILE)

    with pytest.raises(RuntimeError, match="Invalid XML file"):
        IRImager(pathlib.Path(README_FILE))

    with pytest.raises(RuntimeError, match="Failed to open file"):
        IRImager(pathlib.Path("this-file-does-not-exist"))


def test_get_frame_fails_when_not_streaming():
    """Calling `get_frame()` should raise an error when not streaming"""
    irimager = IRImager(XML_FILE)

    with pytest.raises(RuntimeError, match="IRIMAGER_STREAMOFF"):
        irimager.get_frame()


def test_get_frame_in_context_manager():
    """Calling `get_frame()` should work when starting streaming with `with`"""
    irimager = IRImager(XML_FILE)

    # context manager should auto-start streaming
    with irimager:
        irimager.get_frame()

    # context manager should auto-stop streaming
    with pytest.raises(RuntimeError, match="IRIMAGER_STREAMOFF"):
        irimager.get_frame()


def test_context_manager_handles_error():
    """Tests whether the `__exit__` dunder can handle an error"""
    irimager = IRImager(XML_FILE)

    with pytest.raises(RuntimeError, match="Test Exception, should be thrown"):
        with irimager:
            raise RuntimeError("Test Exception, should be thrown")


def test_irimager_get_frame():
    """Tests nqm.irimager.IRImager#get_frame"""
    irimager = IRImager(XML_FILE)

    with irimager:
        array, timestamp = irimager.get_frame()

        assert array.dtype == np.uint16
        # should be 2-dimensional
        assert array.ndim == 2
        assert array.shape == (382, 288)
        assert array.flags["C_CONTIGUOUS"]  # check if the array is row-major

        # image should have been taken in the last 30 seconds
        assert timestamp > datetime.datetime.now() - datetime.timedelta(seconds=30)


def test_irimager_get_temp_range_decimal():
    """Tests that nqm.irimager.IRImager#get_temp_range_decimal returns an int"""
    irimager = IRImager(XML_FILE)

    assert irimager.get_temp_range_decimal() >= 0
    assert isinstance(irimager.get_temp_range_decimal(), int)


def test_irimager_get_library_version():
    """Tests that nqm.irimager.IRImager#get_library_version returns a string"""
    irimager = IRImager(XML_FILE)

    assert isinstance(irimager.get_library_version(), str)
