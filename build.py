"""Poetry build script that builds the nqm.irimager Python C++ extension
"""
from pybind11.setup_helpers import Pybind11Extension, build_ext

nqm_irimager_extension = Pybind11Extension(
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
            "cmdclass": {"build_ext": build_ext},
        }
    )
