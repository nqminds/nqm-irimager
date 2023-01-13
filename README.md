# nqm.irimager

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
