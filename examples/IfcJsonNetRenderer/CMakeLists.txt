cmake_minimum_required(VERSION 3.20)

PROJECT(IfcJsonNetRenderer)

# Set CMake policies to avoid warnings from external dependencies
cmake_policy(SET CMP0144 NEW)  # find_package uses upper-case <PACKAGENAME>_ROOT variables
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)  # The FindBoost module is removed
endif()

# Set policy defaults for external dependencies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)  # option() honors normal variables
set(CMAKE_POLICY_DEFAULT_CMP0144 NEW)  # find_package uses upper-case variables

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

# Fetch Crow framework
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# Temporarily disable jsonnet to avoid CMake compatibility issues
# TODO: Re-enable jsonnet once CMake compatibility is resolved
# For now, the application will work with basic JSON functionality via nlohmann/json
option(ENABLE_JSONNET "Enable Jsonnet templating support" OFF)

if(ENABLE_JSONNET)
    # Jsonnet support disabled due to CMake compatibility issues
    message(STATUS "Jsonnet support is currently disabled due to CMake compatibility issues")
    message(STATUS "Use -DENABLE_JSONNET=ON to attempt enabling it (may fail)")
    
    # Disable jsonnet tests and utilities to avoid googletest dependency
    set(BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
    set(BUILD_JSONNET OFF CACHE BOOL "Build jsonnet executable" FORCE)
    set(BUILD_JSONNETFMT OFF CACHE BOOL "Build jsonnetfmt executable" FORCE)
    set(BUILD_STATIC_LIB ON CACHE BOOL "Build static library" FORCE)
    
    FetchContent_Declare(
        jsonnet
        GIT_REPOSITORY https://github.com/google/jsonnet.git
        GIT_TAG v0.20.0
    )
    FetchContent_MakeAvailable(jsonnet)
endif()

# Fetch nlohmann/json for JSON handling
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/IfcPlusPlus/Debug)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/IfcPlusPlus/${CMAKE_BUILD_TYPE})

ADD_EXECUTABLE(IfcJsonNetRenderer 
    ${IFCPP_SOURCE_DIR}/examples/IfcJsonNetRenderer/src/main.cpp
    ${IFCPP_SOURCE_DIR}/examples/IfcJsonNetRenderer/src/ifc_parser.cpp
    ${IFCPP_SOURCE_DIR}/examples/IfcJsonNetRenderer/src/jsonnet_renderer.cpp
    ${IFCPP_SOURCE_DIR}/examples/IfcJsonNetRenderer/src/rest_endpoints.cpp
)

set_target_properties(IfcJsonNetRenderer PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(IfcJsonNetRenderer PROPERTIES CXX_STANDARD 17)

TARGET_LINK_LIBRARIES(IfcJsonNetRenderer 
    optimized IfcPlusPlus debug IfcPlusPlusd
    Crow::Crow
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Link jsonnet only if enabled
if(ENABLE_JSONNET)
    target_link_libraries(IfcJsonNetRenderer jsonnet_static)
    target_compile_definitions(IfcJsonNetRenderer PRIVATE ENABLE_JSONNET)
endif()

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
)

# Add jsonnet include directory only if enabled
if(ENABLE_JSONNET)
    target_include_directories(IfcJsonNetRenderer PRIVATE ${jsonnet_SOURCE_DIR}/include)
endif()
        
INSTALL(
    TARGETS IfcJsonNetRenderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)