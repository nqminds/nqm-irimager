# This file was auto-generated and modified by hand from
# `pdm run stubgen -m nqm.irimager`

"""Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.
"""
import datetime
import os
import types
import typing

import numpy as np
import numpy.typing as npt

# TODO: replace with PEP 673 typing.Self once we support Python 3.11
_SelfIRImager = typing.TypeVar("_SelfIRImager", bound="IRImager")

__version__: str
"""`nqm.irimager` version.

This is *not* the version of the underlying C++ libirimager library.
"""

def monotonic_to_system_clock(
    steady_time_point: datetime.timedelta,
) -> datetime.datetime:
    """
    Converts from `steady_clock` to `system_clock`.

    Converts a time_point from std::chrono::steady_clock (time since last boot)
    to std::chrono::system_clock (aka time since UNIX epoch).

    C++20 has a function called std::chrono::clock_cast that will do this
    for us, but we're stuck on C++17, so instead we have to do this imprecise
    monstrosity to do the conversion.

    Remarks:
        This function is imprecise!!! Calling it multiple times with the same
        data will result in different results.

    Warning:
        The monotonic/steady_clock might only count when the computer is powered
        on. E.g. if the system was in a sleep state, the monotonic time may not
        have increased. Because of this, you should not rely on this function
        to return accurate results for past time points.
    """

class IRImager:
    """IRImager object - interfaces with a camera."""

    def __init__(self, xml_path: os.PathLike) -> None:
        """Loads the configuration for an IR Camera from the given XML file"""
    def start_streaming(self) -> None:
        """Start video grabbing

        Raises:
            RuntimeError: If streaming cannot be started, e.g. if the camera is not connected.
        """
    def stop_streaming(self) -> None:
        """Stop video grabbing"""
    def __enter__(self: _SelfIRImager) -> _SelfIRImager: ...
    def __exit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc: typing.Optional[BaseException],
        traceback: typing.Optional[types.TracebackType],
    ) -> None: ...
    def get_frame(self) -> typing.Tuple[npt.NDArray[np.uint16], datetime.datetime]:
        """Return a frame

        If the shutter is down (normally done automatically by the thermal
        camera for calibration), this function will wait until the shutter is
        back up, before returning (usually around ~1s).

        Throws:
            RuntimeError if a frame cannot be loaded, e.g. if the camera isn't
            streaming.

        Returns:
            A tuple containing:
              1. A 2-D matrix containing the image. This must be adjusted by
                 :py:meth:`~IRImager.get_temp_range_decimal` to get the actual
                 temperature in degrees Celcius, offset from -100 ℃.
              2. The approximate time the image was taken.
        """
    def get_frame_monotonic(
        self,
    ) -> typing.Tuple[npt.NDArray[np.uint16], datetime.timedelta]:
        """Return a frame, with a monotonic/steady_clock timestamp.

        Similar to :py:meth:`get_frame`, except returns a monotonic timepoint that the
        IRImagerDirectSDK returns, which is more accurate.

        Please be aware that the epoch of the monotonic timepoint is undefined,
        and may be the time since last boot or the time since the program
        started.
        """
    def get_temp_range_decimal(self) -> int:
        """The number of decimal places in the thermal data

        For example, if :py:meth:`~IRImager.get_frame` returns 19000, you can
        divide this number by 10 to the power of the result of
        :py:meth:`~IRImager.get_temp_range_decimal`, then subtract 100,
        to get the actual temperature in degrees Celcius.
        """
    def get_library_version(self) -> typing.Union[str, typing.Literal["MOCKED"]]:
        """Get the version of the libirimager library.

        Returns:
            the version of the libirmager library, or "MOCKED" if the library
            has been mocked.
        """

class IRImagerMock(IRImager):
    """Mocked version of IRImager.

    This class can be used to return dummy data when there isn't a camera
    connected (e.g. for testing).
    """

class Logger:
    """Handles converting C++ logs to Python :py:class:`logging.Logger`.

    After you instantiate an object of this class, all spdlogs will no longer
    be printed to ``stderr``. Instead, they'll go to callback you've defined,
    or a :py:class:`logging.Logger`.

    Additionally, evo::IRLogger logs will also be captured.

    Only a single instance of this object can exist at a time.
    You must destroy existing instances to create a new instance.
    """

    def __init__(self) -> None:
        """Creates a new logger using the default Python :py:class:`logging.Logger`"""
    def __del__(self): ...

class LoggerContextManager:
    """Context Manager around a Logger object.

    Designed for use with Python's ``with`` syntax.
    """

    def __enter__(self) -> None:
        """Starts the logger if it's not already started."""
    def __exit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc: typing.Optional[BaseException],
        traceback: typing.Optional[types.TracebackType],
    ) -> None:
        """Stops the logger"""
