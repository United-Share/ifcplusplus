# Tests CMakeLists.txt für IfcJsonNetRenderer
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

# Test executable
add_executable(IfcJsonNetRenderer_tests
    test_main.cpp
    test_ifc_parser.cpp
    test_jsonnet_renderer.cpp
    test_rest_endpoints.cpp
    test_integration.cpp
)

# Include directories
target_include_directories(IfcJsonNetRenderer_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../IfcPlusPlus/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../IfcPlusPlus/src/ifcpp/IFC4X3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../IfcPlusPlus/src/external
    ${jsonnet_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(IfcJsonNetRenderer_tests
    PRIVATE
    Catch2::Catch2WithMain
    optimized IfcPlusPlus debug IfcPlusPlusd
    Crow::Crow
    jsonnet_static
    Threads::Threads
)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(IfcJsonNetRenderer_tests)

# Enable code coverage if requested
option(CODE_COVERAGE "Enable code coverage" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(IfcJsonNetRenderer_tests PRIVATE --coverage)
    target_link_options(IfcJsonNetRenderer_tests PRIVATE --coverage)
endif()