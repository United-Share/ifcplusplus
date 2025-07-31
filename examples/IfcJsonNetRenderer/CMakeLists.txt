CMAKE_MINIMUM_REQUIRED (VERSION 3.7.2)

PROJECT(IfcJsonNetRenderer)

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(NOT WIN32)
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(NOT WIN32)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DIFCQUERY_STATIC_LIB)
ADD_DEFINITIONS(-DCROW_ENABLE_COMPRESSION=0)
ADD_DEFINITIONS(-DBOOST_ASIO_NO_DEPRECATED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Fetch Crow framework - use master branch for better compatibility
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG master
)

# Disable Crow features we don't need
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "Build Crow examples")
set(CROW_BUILD_TESTS OFF CACHE BOOL "Build Crow tests")
set(CROW_ENABLE_SSL OFF CACHE BOOL "Enable SSL")
set(CROW_ENABLE_COMPRESSION OFF CACHE BOOL "Enable compression")

FetchContent_MakeAvailable(crow)

# Fetch jsonnet header-only library
FetchContent_Declare(
    jsonnet
    GIT_REPOSITORY https://github.com/google/jsonnet.git
    GIT_TAG v0.20.0
)

# Disable googletest for jsonnet to avoid download issues
set(BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(BUILD_GOOGLETEST OFF CACHE BOOL "Build googletest" FORCE)

FetchContent_MakeAvailable(jsonnet)

# Set up paths
get_filename_component(IFCPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(IFCPP_SOURCE_DIR ${IFCPP_ROOT})

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../../IfcPlusPlus/Debug)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../../IfcPlusPlus/${CMAKE_BUILD_TYPE})

ADD_EXECUTABLE(IfcJsonNetRenderer 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ifc_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsonnet_renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rest_endpoints.cpp
)

set_target_properties(IfcJsonNetRenderer PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(IfcJsonNetRenderer PROPERTIES CXX_STANDARD 17)

TARGET_LINK_LIBRARIES(IfcJsonNetRenderer 
    optimized IfcPlusPlus debug IfcPlusPlusd
    Crow::Crow
    jsonnet_static
    Threads::Threads
)

TARGET_INCLUDE_DIRECTORIES(IfcJsonNetRenderer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IFCPP_ROOT}/IfcPlusPlus/src
    ${IFCPP_ROOT}/IfcPlusPlus/src/ifcpp/IFC4X3/include
    ${IFCPP_ROOT}/IfcPlusPlus/src/external
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/glm
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/Carve/include
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/Carve/src
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/Carve/src/include
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/Carve/src/common
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/Carve/build/src
    ${IFCPP_ROOT}/IfcPlusPlus/src/external/glm
    ${jsonnet_SOURCE_DIR}/include
    ${jsonnet_SOURCE_DIR}/third_party/json
    ${jsonnet_SOURCE_DIR}/third_party/json/single_include
)
        
INSTALL(
    TARGETS IfcJsonNetRenderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)
