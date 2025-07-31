// test_ifc_parser.cpp - Tests für IFC Parser
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ifc_parser.h"
#include <fstream>
#include <filesystem>

using Catch::Matchers::Contains;
namespace fs = std::filesystem;

// Test-Fixtures für wiederverwendbare Test-Setups
class IfcParserTestFixture {
protected:
    std::unique_ptr<IfcParser> parser;
    std::string test_ifc_path;
    
    IfcParserTestFixture() {
        parser = std::make_unique<IfcParser>();
        test_ifc_path = "test_data/simple_wall.ifc";
    }
    
    void createSimpleIfcFile() {
        fs::create_directories("test_data");
        std::ofstream file(test_ifc_path);
        file << "ISO-10303-21;\n"
             << "HEADER;\n"
             << "FILE_DESCRIPTION(('ViewDefinition [CoordinationView]'),'2;1');\n"
             << "FILE_NAME('simple_wall.ifc','2024-01-01T00:00:00',(),(),'IfcPlusPlus','IfcPlusPlus','');\n"
             << "FILE_SCHEMA(('IFC4X3'));\n"
             << "ENDSEC;\n"
             << "DATA;\n"
             << "#1=IFCPROJECT('1234567890123456',$,'Test Project',$,$,$,$,$,#2);\n"
             << "#2=IFCGEOMETRICREPRESENTATIONCONTEXT($,'Model',3,1.0E-05,#3,$);\n"
             << "#3=IFCAXIS2PLACEMENT3D(#4,$,$);\n"
             << "#4=IFCCARTESIANPOINT((0.,0.,0.));\n"
             << "#5=IFCWALL('2234567890123456',$,'Test Wall',$,$,#6,$,$,$);\n"
             << "#6=IFCLOCALPLACEMENT($,#7);\n"
             << "#7=IFCAXIS2PLACEMENT3D(#8,$,$);\n"
             << "#8=IFCCARTESIANPOINT((0.,0.,0.));\n"
             << "ENDSEC;\n"
             << "END-ISO-10303-21;\n";
        file.close();
    }
    
    ~IfcParserTestFixture() {
        // Aufräumen
        if (fs::exists("test_data")) {
            fs::remove_all("test_data");
        }
    }
};

// Test-Driven Development: Zuerst die Tests schreiben

TEST_CASE("IfcParser Basic Functionality", "[ifc_parser]") {
    IfcParserTestFixture fixture;
    
    SECTION("Parser can be instantiated") {
        REQUIRE(fixture.parser != nullptr);
    }
    
    SECTION("Loading non-existent file throws exception") {
        REQUIRE_THROWS_AS(
            fixture.parser->loadFile("non_existent.ifc"),
            std::runtime_error
        );
    }
    
    SECTION("Loading valid IFC file succeeds") {
        fixture.createSimpleIfcFile();
        REQUIRE_NOTHROW(fixture.parser->loadFile(fixture.test_ifc_path));
        REQUIRE(fixture.parser->isLoaded());
    }
}

TEST_CASE("IfcParser JSON Conversion", "[ifc_parser][json]") {
    IfcParserTestFixture fixture;
    fixture.createSimpleIfcFile();
    fixture.parser->loadFile(fixture.test_ifc_path);
    
    SECTION("Convert to JSON produces valid output") {
        auto json_result = fixture.parser->toJson();
        
        REQUIRE(json_result.is_object());
        REQUIRE(json_result.contains("project"));
        REQUIRE(json_result.contains("entities"));
        REQUIRE(json_result.contains("geometry"));
    }
    
    SECTION("Project information is correctly extracted") {
        auto json_result = fixture.parser->toJson();
        auto project = json_result["project"];
        
        REQUIRE(project["name"] == "Test Project");
        REQUIRE(project["guid"] == "1234567890123456");
    }
    
    SECTION("Entities are correctly counted") {
        auto json_result = fixture.parser->toJson();
        auto entities = json_result["entities"];
        
        REQUIRE(entities["total_count"].get<int>() > 0);
        REQUIRE(entities["by_type"].contains("IfcWall"));
        REQUIRE(entities["by_type"]["IfcWall"]["count"] == 1);
    }
}

TEST_CASE("IfcParser Entity Queries", "[ifc_parser][query]") {
    IfcParserTestFixture fixture;
    fixture.createSimpleIfcFile();
    fixture.parser->loadFile(fixture.test_ifc_path);
    
    SECTION("Get entities by type") {
        auto walls = fixture.parser->getEntitiesByType("IfcWall");
        
        REQUIRE(walls.is_array());
        REQUIRE(walls.size() == 1);
        REQUIRE(walls[0]["name"] == "Test Wall");
    }
    
    SECTION("Get entity by GUID") {
        auto entity = fixture.parser->getEntityByGuid("2234567890123456");
        
        REQUIRE(entity.is_object());
        REQUIRE(entity["type"] == "IfcWall");
        REQUIRE(entity["name"] == "Test Wall");
    }
    
    SECTION("Get non-existent entity returns null") {
        auto entity = fixture.parser->getEntityByGuid("non_existent_guid");
        
        REQUIRE(entity.is_null());
    }
}

TEST_CASE("IfcParser Geometry Extraction", "[ifc_parser][geometry]") {
    IfcParserTestFixture fixture;
    fixture.createSimpleIfcFile();
    fixture.parser->loadFile(fixture.test_ifc_path);
    
    SECTION("Extract geometry for entity") {
        auto geometry = fixture.parser->getGeometry("2234567890123456");
        
        REQUIRE(geometry.is_object());
        REQUIRE(geometry.contains("meshes"));
        REQUIRE(geometry["meshes"].is_array());
    }
    
    SECTION("Geometry contains transformation matrix") {
        auto geometry = fixture.parser->getGeometry("2234567890123456");
        
        REQUIRE(geometry.contains("transform"));
        REQUIRE(geometry["transform"].is_array());
        REQUIRE(geometry["transform"].size() == 16); // 4x4 matrix
    }
}

TEST_CASE("IfcParser Building Hierarchy", "[ifc_parser][hierarchy]") {
    IfcParserTestFixture fixture;
    fixture.createSimpleIfcFile();
    fixture.parser->loadFile(fixture.test_ifc_path);
    
    SECTION("Get building hierarchy") {
        auto hierarchy = fixture.parser->getBuildingHierarchy();
        
        REQUIRE(hierarchy.is_object());
        REQUIRE(hierarchy["type"] == "IfcProject");
        REQUIRE(hierarchy["name"] == "Test Project");
        REQUIRE(hierarchy.contains("children"));
    }
}

// Performance-Tests mit Catch2 Benchmarks
TEST_CASE("IfcParser Performance", "[!benchmark]") {
    IfcParserTestFixture fixture;
    fixture.createSimpleIfcFile();
    
    BENCHMARK("Load IFC file") {
        IfcParser parser;
        parser.loadFile(fixture.test_ifc_path);
    };
    
    fixture.parser->loadFile(fixture.test_ifc_path);
    
    BENCHMARK("Convert to JSON") {
        return fixture.parser->toJson();
    };
    
    BENCHMARK("Query entities by type") {
        return fixture.parser->getEntitiesByType("IfcWall");
    };
}