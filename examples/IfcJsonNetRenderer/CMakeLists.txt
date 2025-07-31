CMAKE_MINIMUM_REQUIRED (VERSION 3.7.2)

PROJECT(IfcJsonNetRenderer)

# Define IFCPP_SOURCE_DIR if not defined
if(NOT DEFINED IFCPP_SOURCE_DIR)
    set(IFCPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
endif()

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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Fetch Crow framework - use latest version that supports modern Boost
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)

# Set Crow to use standalone asio to avoid Boost compatibility issues
set(CROW_USE_BOOST OFF CACHE BOOL "Use Boost.Asio instead of standalone Asio" FORCE)

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

# nlohmann/json is already included as a dependency of jsonnet
# No need to fetch it separately

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/IfcPlusPlus/Debug)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/IfcPlusPlus/${CMAKE_BUILD_TYPE})

ADD_EXECUTABLE(IfcJsonNetRenderer 
    src/main.cpp
    src/ifc_parser.cpp
    src/jsonnet_renderer.cpp
    src/rest_endpoints.cpp
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
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/ifcpp/IFC4X3/include
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/glm
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/Carve/include
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/Carve/src
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/Carve/src/include
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/Carve/src/common
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/Carve/build/src
    ${IFCPP_SOURCE_DIR}/IfcPlusPlus/src/external/glm
    ${jsonnet_SOURCE_DIR}/include
    ${jsonnet_SOURCE_DIR}/third_party/json
    ${jsonnet_SOURCE_DIR}/third_party
)
        
INSTALL(
    TARGETS IfcJsonNetRenderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)