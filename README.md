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
