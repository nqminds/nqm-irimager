# This file was auto-generated and modified by hand from
# `poetry run stubgen -m nqm.irimager`

"""Optris PI and XI imager IR camera controller

We use the IRImagerDirect SDK
(see http://documentation.evocortex.com/libirimager2/html/index.html)
to control these cameras.
"""

class IRImager:
    """IRImager object - interfaces with a camera."""

    def __init__(self, *args, **kwargs) -> None: ...
    def test(self) -> int:
        """Return the number 42"""
