# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Add `nqm.logger.LoggerContextManager` Python class, which automatically
  creates/deletes a
  [`nqm.logger.Logger`](https://nqminds.github.io/nqm-irimager/apidoc/nqm.irimager.html#nqm.irimager.Logger)
  object when used in a [`with:` statement][PEP 343] ([#81][]).
- Add `nqm.irimager.IRImager.get_frame_monotonic` Python method, which can be
  used to get the monotonic time of a frame directly from the
  EvoCortex IRImagerDirect SDK ([#84][]).
- Add `nqm.irimager.monotonic_to_system_clock` function to convert a monotonic
  time to a system clock time ([#84][]).

### Fixed

- Truncate timestamps to
  [`std::chrono::steady_clock::duration`](https://en.cppreference.com/w/cpp/chrono/steady_clock)
  instead of
  [`std::chrono::nanoseconds`](https://en.cppreference.com/w/cpp/chrono/duration)
  ([#86][]).

[#81]: https://github.com/nqminds/nqm-irimager/pull/81
[#84]: https://github.com/nqminds/nqm-irimager/pull/84
[#86]: https://github.com/nqminds/nqm-irimager/pull/86
[PEP 343]: https://peps.python.org/pep-0343/

## [1.0.0] - 2023-10-30

Initial release.

[unreleased]: https://github.com/nqminds/nqm-irimager/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/nqminds/nqm-irimager/releases/tag/v1.0.0
