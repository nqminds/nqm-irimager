"""Tests for nqm.irimager.IRImager"""
from nqm.irimager import IRImager


def test_irimager_test():
    """Tests nqm.irimager.IRImager"""
    irimager = IRImager()

    assert irimager.test() == 42
