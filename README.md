# nqm.irimager

[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)
[![Linting: Pylint](https://img.shields.io/badge/linting-pylint-yellowgreen)](https://github.com/PyCQA/pylint)
[![Imports: isort](https://img.shields.io/badge/%20imports-isort-%231674b1?style=flat&labelColor=ef8336)](https://pycqa.github.io/isort/)

Python module for interfacing with [EvoCortex IRImagerDirect SDK][1].

[1]: https://evocortex.org/products/irimagerdirect-sdk/

## Setup

This repo uses [Python poetry](https://github.com/python-poetry/poetry) to
manage Python dependencies. You can install dependencies by running:

```bash
poetry install
```

## Development

### Pre-commit checks (linting and type checks)

This project uses [`pre-commit`](https://pre-commit.com/) to check code.
You may wish to run `poetry run pre-commit install` to automatically run
checks before you run `git commit`.

Additionally, you can run `poetry run pre-commit run --all` to check all changes.

### Tests

Tests are written using [`pytest`](https://docs.pytest.org/en/7.2.x/), and can be run with:

```bash
poetry run pytest
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
me@user:~$ MYPYPATH=src poetry run stubtest nqm.irimager
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
poetry run sphinx-apidoc --implicit-namespaces -o docs/apidoc src/nqm/
poetry run sphinx-build -M html docs/ build/
```
