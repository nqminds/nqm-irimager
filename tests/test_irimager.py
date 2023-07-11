"""Tests for nqm.irimager.IRImager"""
import datetime

import numpy as np
import pytest

from nqm.irimager import IRImager


def test_irimager_test():
    """Tests nqm.irimager.IRImager"""
    irimager = IRImager()

    assert irimager.test() == 42


def test_irimager_get_frame():
    """Tests nqm.irimager.IRImager#get_frame"""
    irimager = IRImager()

    with pytest.raises(RuntimeError, match="IRIMAGER_STREAMOFF"):
        array, timestamp = irimager.get_frame()

    irimager.start_streaming()
    array, timestamp = irimager.get_frame()

    assert array.dtype == np.uint16
    # should be 2-dimensional
    assert array.ndim == 2
    assert array.shape == (128, 128)

    # image should have been taken in the last 30 seconds
    assert timestamp > datetime.datetime.now() - datetime.timedelta(seconds=30)
