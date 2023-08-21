"""Tests for nqm.irimager.__version__"""
import pathlib
import sys

if sys.version_info < (3, 11):
    import tomli as tomllib
else:
    import tomllib

import nqm.irimager


def test_version():
    """Test whether the PEP 396 __version__ attribute is correctly set"""
    with (pathlib.Path(__file__).parent.parent / "pyproject.toml").open(
        "rb"
    ) as pyproject_file:
        data = tomllib.load(pyproject_file)
    expected_version = data["project"]["version"]
    actual_version = nqm.irimager.__version__

    assert actual_version == expected_version
