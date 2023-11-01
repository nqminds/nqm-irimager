# nqm.irimager

[![PyPI - Version](https://img.shields.io/pypi/v/nqm.irimager)](https://pypi.org/project/nqm.irimager/)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)
[![Linting: Pylint](https://img.shields.io/badge/linting-pylint-yellowgreen)](https://github.com/PyCQA/pylint)
[![Imports: isort](https://img.shields.io/badge/%20imports-isort-%231674b1?style=flat&labelColor=ef8336)](https://pycqa.github.io/isort/)
[![Managed by PDM](https://img.shields.io/badge/pdm-managed-blueviolet)](https://pdm.fming.dev/)
[![Documentation on GitHub Pages](https://github.com/nqminds/nqm-irimager/actions/workflows/docs.yml/badge.svg?branch=main)](https://nqminds.github.io/nqm-irimager/)
[![Test GitHub Action](https://github.com/nqminds/nqm-irimager/actions/workflows/test.yml/badge.svg)](https://github.com/nqminds/nqm-irimager/actions/workflows/test.yml)

Python module for interfacing with [EvoCortex IRImagerDirect SDK][1].

[1]: https://evocortex.org/products/irimagerdirect-sdk/

## Setup

`nqm.irimager` uses the [EvoCortex IRImagerDirect SDK][1]
(also known as `libirimager`).

Before using `nqm.irimager`, you'll need to perform the following setup for
each camera you have:

- Download calibration data using `sudo ir_download_calibration`, see
  <http://documentation.evocortex.com/libirimager2/html/Installation.html#subsec_download>
- Create an XML configuration file using `ir_generate_configuration`, see
  <http://documentation.evocortex.com/libirimager2/html/Installation.html#subsec_config>

### Installing pre-built wheel from PyPI

`nqm.irimager` publishes pre-built binary wheels for a couple of targets to
[PyPI][], so you may just be able to run:

```bash
pip install nqm.irimager
```

[PyPI]: https://pypi.org/project/nqm.irimager/

### Compiling your own wheel

You can compile `nqm.irimager` from source for testing, or if you want to use
a target that we don't yet publish binaries for.

You'll need a compiler that supports at least C++17.

#### Installing `libirimager`

`nqm.irimager` requires the [EvoCortex IRImagerDirect SDK][1]
(known as `libirimager`). Please follow the instructions on their webpage to
install the package.

It's possible to install a mocked version of `nqm.irimager` for testing
by defining `SKBUILD_CMAKE_DEFINE='IRImager_mock=ON'` whiling building
`nqm.irimager`.

#### Install `nqm.irimager`

This repo uses [PEP621](https://peps.python.org/pep-0621/) to manage
dependencies. [scikit-build-core](https://pypi.org/project/scikit-build-core/)
is used to compile the project as a `build-system.build-backend`.

We recommend using [PDM](https://pdm.fming.dev/latest/) for local development.

```bash
pdm install
```

## Usage example

See documentation for the full API reference, but using this library is as
easy as:

```python
import datetime
import logging
from nqm.irimager import IRImager, LoggerContextManager

logging.basicConfig()
logging.getLogger().setLevel(1) # trace

# Your XML config,
# see http://documentation.evocortex.com/libirimager2/html/Overview.html#subsec_overview_config_file
XML_CONFIG = "tests/__fixtures__/382x288@27Hz.xml"
with LoggerContextManager():
  irimager = IRImager(XML_CONFIG)
  with irimager:
    print(f"Started at {datetime.datetime.now()}")
    while True: # press CTRL+C to stop this program
        try:
          array, timestamp = irimager.get_frame()
        except error:
          print(f"Stopped at {datetime.datetime.now()}")
          raise error
        frame_in_celsius = array / (10 ** irimager.get_temp_range_decimal()) - 100
        print(f"At {timestamp}: Average temperature is {frame_in_celsius.mean()}")
```

## Development

### Pre-commit checks (linting and type checks)

This project uses [`pre-commit`](https://pre-commit.com/) to check code.
You may wish to run `pdm run pre-commit install` to automatically run
checks before you run `git commit`.

Additionally, you can run `pdm run pre-commit run --all` to check all changes.

### Tests

Tests are written using [`pytest`](https://docs.pytest.org/en/7.2.x/), and can be run with:

```bash
pdm run pytest
```

#### C++ unit tests

Some C++ tests are written using
[`GoogleTest`](https://google.github.io/googletest/) and
[`gMock`](https://google.github.io/googletest/gmock_for_dummies.html).

As these tests need the [`Development.Embed`][FindPython3] Python3 embedding
artifact (which aren't available on most systems), you must opt-in to building
and running these tests by using:

```bash
SKBUILD_CMAKE_DEFINE="BUILD_TESTING=ON" pdm install && pdm run pytest
```

If you want to use both `BUILD_TESTING=ON` and `IRImager_mock=ON`, you can do the following:

```bash
SKBUILD_CMAKE_DEFINE='BUILD_TESTING=ON;IRImager_mock=ON' pdm install && pdm run pytest
```

#### Mypy stubtest

You can use
[Mypy's `stubtest` tool](https://mypy.readthedocs.io/en/stable/stubtest.html)
to automatically check whether the types in a stub file match the
implementation.

For most Python code, we put the type information directly in the
implementation, so we only need this for C/C++ Python extensions, like the
`nqm.irimager` module.

> **Note**
>
> The `error: nqm.irimager.IRImager is inconsistent, metaclass differs` errors
> are expected, due to the inability to stub built-in pybind11 types.

```bash
me@user:~$ MYPYPATH=src .venv/bin/stubtest nqm.irimager
error: nqm.irimager.IRImager is inconsistent, metaclass differs
Stub: in file src/nqm/irimager/__init__.pyi:11
N/A
Runtime:
<class 'pybind11_builtins.pybind11_type'>

Found 1 error (checked 1 module)
```

### Documentation

[Sphinx](https://www.sphinx-doc.org/en/master/index.html) is used to generate
documentation in the `docs/` directory.

[sphinx.ext.napoleon](https://www.sphinx-doc.org/en/master/usage/extensions/napoleon.html)
automatically parses Google-style Python docstrings.

```bash
pdm run sphinx-build -M html docs/ build/
```

### Publishing a new release

To publish a new release to PyPI, do:

1. Make a new commit called `chore: release v1.X.X`
   This should change:
    - The `project.version` field in the `pyproject.toml` file (e.g. https://github.com/nqminds/nqm-irimager/blob/47c1b941df8ecfabcc96fc6d0f17120ec6105671/pyproject.toml#L3)
    - Make a new entry in the `CHANGELOG.md` file, moving any appropriate entries
      from the `Unreleased` section.
2. Push the commit to GitHub.
3. Make a new
   [GitHub release](https://github.com/nqminds/nqm-irimager/releases) for the commit you made, copying the CHANGELOG.md entry for the release notes.
4. The [`build.yml` GitHub Action](https://github.com/nqminds/nqm-irimager/actions/workflows/build.yml?query=event%3Arelease) should automatically build and publish
   the new release to PyPI.
5. Merge the released commit to the `main` branch if you haven't already.
