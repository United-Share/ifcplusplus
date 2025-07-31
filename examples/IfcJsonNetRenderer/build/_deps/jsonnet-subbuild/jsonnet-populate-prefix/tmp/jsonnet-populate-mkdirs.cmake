# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-src")
  file(MAKE_DIRECTORY "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-src")
endif()
file(MAKE_DIRECTORY
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build"
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix"
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/tmp"
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/src/jsonnet-populate-stamp"
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/src"
  "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/src/jsonnet-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/src/jsonnet-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-subbuild/jsonnet-populate-prefix/src/jsonnet-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
