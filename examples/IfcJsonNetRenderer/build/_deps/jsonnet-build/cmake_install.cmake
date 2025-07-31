# Install script for directory: /workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/third_party/json/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/include/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/stdlib/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/third_party/md5/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/ryml/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/core/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/cpp/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/cmd/cmake_install.cmake")
  include("/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/test_suite/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/workspace/examples/IfcJsonNetRenderer/build/_deps/jsonnet-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
