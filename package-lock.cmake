# CPM Package Lock
# This file should be committed to version control

# spdlog
CPMDeclarePackage(spdlog
  NAME spdlog
  VERSION 1.12.0
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
