# CPM Package Lock
# This file should be committed to version control

# googletest
CPMDeclarePackage(googletest
  NAME googletest
  VERSION 1.14.0
  EXCLUDE_FROM_ALL TRUE URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz URL_HASH SHA3_256=d00154b480c96478b7d06491a536a4d99cd0eabcb8c654dec2e58412d8fca9cc
)
# spdlog
CPMDeclarePackage(spdlog
  NAME spdlog
  VERSION 1.12.0
  SYSTEM ON
  OPTIONS
    "SPDLOG_BUILD_PIC ON"
    "URL"
    "https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.tar.gz"
    "URL_HASH"
    "SHA3_256=951142aefc65510834d3731872bfce9186dc4d18682a773b7b7f487dd08653e1"
)
# propagate_const
CPMDeclarePackage(propagate_const
  NAME propagate_const
  VERSION 1.0.0
  URL https://github.com/jbcoe/propagate_const/archive/672cdbcd27028f8985dc7a559a085e530ac4656b.tar.gz URL_HASH SHA3_256=bd64b6f3886e223bdea0bfc53182f686bbae2b27ff5ea4b4a85342be78747cd4
)
# eigen
CPMDeclarePackage(eigen
  NAME eigen
  VERSION 3.4.0
  SYSTEM ON
  EXCLUDE_FROM_ALL TRUE URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.bz2 URL_HASH SHA3_256=652266ca1f8c15663076d21be840a85910ae4071453b2a1aa6f6a55ea22daddf
)
