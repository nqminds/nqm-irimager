"""Poetry build script that builds the nqm.irimager Python C++ extension
"""

from setuptools.extension import Extension

nqm_irimager_extension = Extension(
    "nqm.irimager",
    sources=["src/nqm/irimager/irimager.cpp"],
    define_macros=[("PY_SSIZE_T_CLEAN", None)],
)


def build(setup_kwargs):
    """
    This is a callback for poetry used to hook in our extensions.
    """

    setup_kwargs.update(
        {
            # declare the extension so that setuptools will compile it
            "ext_modules": [nqm_irimager_extension],
        }
    )
