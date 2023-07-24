# nqm.irimager

[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)
[![Linting: Pylint](https://img.shields.io/badge/linting-pylint-yellowgreen)](https://github.com/PyCQA/pylint)
[![Imports: isort](https://img.shields.io/badge/%20imports-isort-%231674b1?style=flat&labelColor=ef8336)](https://pycqa.github.io/isort/)
[![Managed by PDM](https://img.shields.io/badge/pdm-managed-blueviolet)](https://pdm.fming.dev/)

Python module for interfacing with [EvoCortex IRImagerDirect SDK][1].

[1]: https://evocortex.org/products/irimagerdirect-sdk/

## Setup

This repo uses [PEP621](https://peps.python.org/pep-0621/) to manage
dependencies. [scikit-build-core](https://pypi.org/project/scikit-build-core/)
is used to compile the project as a `build-system.build-backend`.

We recommend using [PDM](https://pdm.fming.dev/latest/) for local development.

```bash
pdm install
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
pdm run .venv/bin/sphinx-apidoc --implicit-namespaces -o docs/apidoc src/nqm/
pdm run .venv/bin/sphinx-build -M html docs/ build/
```
