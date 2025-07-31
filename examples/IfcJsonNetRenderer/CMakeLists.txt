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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Fetch Crow framework with ASIO standalone
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)

# Configure Crow to use standalone ASIO without Boost
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_FEATURES "ssl" CACHE STRING "" FORCE)
set(ASIO_STANDALONE ON CACHE BOOL "" FORCE)

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
)
        
INSTALL(
    TARGETS IfcJsonNetRenderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)

# Enable testing
option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()