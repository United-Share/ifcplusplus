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

# Create a simple executable without external web framework for now
ADD_EXECUTABLE(IfcJsonNetRenderer 
    src/main_simple.cpp
    src/ifc_parser_simple.cpp
)

set_target_properties(IfcJsonNetRenderer PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(IfcJsonNetRenderer PROPERTIES CXX_STANDARD 17)

TARGET_LINK_LIBRARIES(IfcJsonNetRenderer 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/Release/libIfcPlusPlus.a
    Threads::Threads
)

TARGET_INCLUDE_DIRECTORIES(IfcJsonNetRenderer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/ifcpp/IFC4X3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/Carve/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/Carve/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/Carve/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/Carve/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/Carve/build/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../IfcPlusPlus/src/external/glm
)
        
INSTALL(
    TARGETS IfcJsonNetRenderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)