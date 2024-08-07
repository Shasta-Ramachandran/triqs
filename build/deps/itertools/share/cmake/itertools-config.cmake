# This file allows other CMake Projects to find us
# We provide general project information
# and reestablish the exported CMake Targets

# Multiple inclusion guard
if(NOT itertools_FOUND)
set(itertools_FOUND TRUE)
set_property(GLOBAL PROPERTY itertools_FOUND TRUE)

# version
set(itertools_VERSION 1.3.0 CACHE STRING "itertools version")
set(itertools_GIT_HASH 2bb4da2b56ab49b3cd7b5f27cfb88c5320fe8524 CACHE STRING "itertools git hash")

# Root of the installation
set(itertools_ROOT /Users/ssr38/Documents/GitHub/triqs/install CACHE STRING "itertools root directory")

## Find the target dependencies
#function(find_dep)
#  get_property(${ARGV0}_FOUND GLOBAL PROPERTY ${ARGV0}_FOUND)
#  if(NOT ${ARGV0}_FOUND)
#    find_package(${ARGN} REQUIRED HINTS /Users/ssr38/Documents/GitHub/triqs/install)
#  endif()
#endfunction()
#find_dep(depname 1.0)

# Include the exported targets of this project
include(/Users/ssr38/Documents/GitHub/triqs/install/lib/cmake/itertools/itertools-targets.cmake)

message(STATUS "Found itertools-config.cmake with version 1.3.0, hash = 2bb4da2b56ab49b3cd7b5f27cfb88c5320fe8524, root = /Users/ssr38/Documents/GitHub/triqs/install")

# Was the Project built with Documentation?
set(itertools_WITH_DOCUMENTATION OFF CACHE BOOL "Was itertools build with documentation?")

endif()
