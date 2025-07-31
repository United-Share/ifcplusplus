// test_integration.cpp - Integrationstests für das Gesamtsystem
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ifc_parser.h"
#include "jsonnet_renderer.h"
#include "rest_endpoints.h"
#include <crow.h>
#include <filesystem>
#include <thread>

using Catch::Matchers::Contains;
namespace fs = std::filesystem;

class IntegrationTestFixture {
protected:
    std::unique_ptr<IfcParser> parser;
    std::unique_ptr<JsonnetRenderer> renderer;
    std::unique_ptr<RestEndpoints> endpoints;
    crow::SimpleApp app;
    
    IntegrationTestFixture() {
        parser = std::make_unique<IfcParser>();
        renderer = std::make_unique<JsonnetRenderer>();
        endpoints = std::make_unique<RestEndpoints>();
        
        // Setup der Komponenten
        endpoints->setParser(parser.get());
        endpoints->setRenderer(renderer.get());
        endpoints->setup(app);
        
        createTestData();
    }
    
    void createTestData() {
        fs::create_directories("test_data");
        
        // IFC-Testdatei
        std::ofstream ifc_file("test_data/building.ifc");
        ifc_file << R"(ISO-10303-21;
HEADER;
FILE_DESCRIPTION(('ViewDefinition [CoordinationView]'),'2;1');
FILE_NAME('building.ifc','2024-01-01T00:00:00',(),(),'IfcPlusPlus','IfcPlusPlus','');
FILE_SCHEMA(('IFC4X3'));
ENDSEC;
DATA;
#1=IFCPROJECT('3PczXF7zH9xQgGb45VVfXt',$,'Office Building',$,$,$,$,#2);
#2=IFCGEOMETRICREPRESENTATIONCONTEXT($,'Model',3,1.0E-05,#3,$);
#3=IFCAXIS2PLACEMENT3D(#4,$,$);
#4=IFCCARTESIANPOINT((0.,0.,0.));
#5=IFCBUILDING('1xS3BCk291UvhgP2dvNsgp',$,'Main Building',$,$,#6,$,$,.ELEMENT.,$,$,$);
#6=IFCLOCALPLACEMENT($,#7);
#7=IFCAXIS2PLACEMENT3D(#8,$,$);
#8=IFCCARTESIANPOINT((0.,0.,0.));
#9=IFCBUILDINGSTOREY('0QbiXuKNP2GQXLR_LfLPL_',$,'Ground Floor',$,$,#10,$,$,.ELEMENT.,0.);
#10=IFCLOCALPLACEMENT(#6,#11);
#11=IFCAXIS2PLACEMENT3D(#12,$,$);
#12=IFCCARTESIANPOINT((0.,0.,0.));
#13=IFCWALL('39_HHgBxX3$xPyPLCE88im',$,'Wall 001',$,$,#14,$,$,$);
#14=IFCLOCALPLACEMENT(#10,#15);
#15=IFCAXIS2PLACEMENT3D(#16,$,$);
#16=IFCCARTESIANPOINT((0.,0.,0.));
#17=IFCWALL('1TGeFNNyn1_9kGMbjcAIz6',$,'Wall 002',$,$,#18,$,$,$);
#18=IFCLOCALPLACEMENT(#10,#19);
#19=IFCAXIS2PLACEMENT3D(#20,$,$);
#20=IFCCARTESIANPOINT((5000.,0.,0.));
#21=IFCSLAB('3fGeYNKMD3_gL7PDe1fQID',$,'Floor Slab',$,$,#22,$,$,.FLOOR.);
#22=IFCLOCALPLACEMENT(#10,#23);
#23=IFCAXIS2PLACEMENT3D(#24,$,$);
#24=IFCCARTESIANPOINT((0.,0.,-200.));
#25=IFCRELAGGREGATES('2gOroG8yH8xBBMbCFVfOOH',$,$,$,#1,(#5));
#26=IFCRELAGGREGATES('3xkLXaWtf27BPqI84Tv9sN',$,$,$,#5,(#9));
#27=IFCRELCONTAINEDINSPATIALSTRUCTURE('1oZ9ER$nD4xBK3bH5qTLDK',$,$,$,(#13,#17,#21),#9);
ENDSEC;
END-ISO-10303-21;
)";
        ifc_file.close();
        
        // Jsonnet-Template
        std::ofstream template_file("test_data/analysis.jsonnet");
        template_file << R"({
    local ifc = std.extVar("ifc_data"),
    
    project_summary: {
        name: ifc.project.name,
        building_count: std.length(ifc.hierarchy.children),
        total_entities: ifc.entities.total_count
    },
    
    wall_analysis: {
        count: if std.objectHas(ifc.entities.by_type, "IfcWall") 
               then ifc.entities.by_type.IfcWall.count 
               else 0,
        walls: if std.objectHas(ifc.entities.by_type, "IfcWall") && 
                  std.objectHas(ifc.entities.by_type.IfcWall, "items")
               then [
                   {
                       guid: wall.guid,
                       name: wall.name,
                       analysis: "Wall analysis for " + wall.name
                   }
                   for wall in ifc.entities.by_type.IfcWall.items
               ]
               else []
    },
    
    timestamp: std.extVar("timestamp")
})";
        template_file.close();
    }
    
    ~IntegrationTestFixture() {
        if (fs::exists("test_data")) {
            fs::remove_all("test_data");
        }
    }
};

TEST_CASE("Integration: Full Workflow", "[integration]") {
    IntegrationTestFixture fixture;
    
    SECTION("Load IFC, convert to JSON, render template") {
        // Schritt 1: IFC-Datei laden
        fixture.parser->loadFile("test_data/building.ifc");
        REQUIRE(fixture.parser->isLoaded());
        
        // Schritt 2: In JSON konvertieren
        auto ifc_json = fixture.parser->toJson();
        REQUIRE(ifc_json["project"]["name"] == "Office Building");
        REQUIRE(ifc_json["entities"]["by_type"]["IfcWall"]["count"] == 2);
        
        // Schritt 3: Template rendern
        std::ifstream template_file("test_data/analysis.jsonnet");
        std::string template_content((std::istreambuf_iterator<char>(template_file)),
                                    std::istreambuf_iterator<char>());
        
        std::map<std::string, std::string> vars = {
            {"timestamp", "2024-01-01T12:00:00Z"}
        };
        
        auto result = fixture.renderer->renderTemplate(
            template_content,
            ifc_json,
            vars
        );
        
        REQUIRE(result["project_summary"]["name"] == "Office Building");
        REQUIRE(result["wall_analysis"]["count"] == 2);
        REQUIRE(result["wall_analysis"]["walls"].size() == 2);
        REQUIRE(result["timestamp"] == "2024-01-01T12:00:00Z");
    }
}

TEST_CASE("Integration: Entity Relationships", "[integration][relationships]") {
    IntegrationTestFixture fixture;
    fixture.parser->loadFile("test_data/building.ifc");
    
    SECTION("Building hierarchy is correctly established") {
        auto hierarchy = fixture.parser->getBuildingHierarchy();
        
        REQUIRE(hierarchy["type"] == "IfcProject");
        REQUIRE(hierarchy["name"] == "Office Building");
        REQUIRE(hierarchy["children"].size() == 1);
        
        auto building = hierarchy["children"][0];
        REQUIRE(building["type"] == "IfcBuilding");
        REQUIRE(building["name"] == "Main Building");
        REQUIRE(building["children"].size() == 1);
        
        auto storey = building["children"][0];
        REQUIRE(storey["type"] == "IfcBuildingStorey");
        REQUIRE(storey["name"] == "Ground Floor");
        REQUIRE(storey["contained_elements"].size() == 3);
    }
    
    SECTION("Spatial containment is resolved") {
        auto walls = fixture.parser->getEntitiesByType("IfcWall");
        
        for (const auto& wall : walls) {
            REQUIRE(wall.contains("contained_in"));
            REQUIRE(wall["contained_in"]["type"] == "IfcBuildingStorey");
            REQUIRE(wall["contained_in"]["name"] == "Ground Floor");
        }
    }
}

TEST_CASE("Integration: Error Recovery", "[integration][error]") {
    IntegrationTestFixture fixture;
    
    SECTION("System handles corrupted IFC gracefully") {
        // Erstelle korrupte IFC-Datei
        std::ofstream bad_file("test_data/corrupted.ifc");
        bad_file << "This is not a valid IFC file";
        bad_file.close();
        
        REQUIRE_THROWS(fixture.parser->loadFile("test_data/corrupted.ifc"));
        REQUIRE(!fixture.parser->isLoaded());
        
        // System sollte trotzdem funktionsfähig bleiben
        fixture.parser->loadFile("test_data/building.ifc");
        REQUIRE(fixture.parser->isLoaded());
    }
    
    SECTION("Template errors don't crash the system") {
        fixture.parser->loadFile("test_data/building.ifc");
        auto ifc_json = fixture.parser->toJson();
        
        std::string bad_template = R"({
            error: std.extVar("non_existent").value
        })";
        
        auto result = fixture.renderer->renderTemplate(bad_template, ifc_json);
        REQUIRE(result.contains("error"));
        
        // System sollte weiterhin funktionieren
        std::string good_template = R"({
            project: std.extVar("ifc_data").project.name
        })";
        
        result = fixture.renderer->renderTemplate(good_template, ifc_json);
        REQUIRE(result["project"] == "Office Building");
    }
}

TEST_CASE("Integration: Performance", "[integration][performance]") {
    IntegrationTestFixture fixture;
    
    SECTION("Large IFC processing") {
        // Erstelle größere IFC-Datei
        std::ofstream large_file("test_data/large.ifc");
        large_file << "ISO-10303-21;\n"
                   << "HEADER;\n"
                   << "FILE_DESCRIPTION(('Large Building'),'2;1');\n"
                   << "FILE_NAME('large.ifc','2024-01-01T00:00:00',(),(),'Test','Test','');\n"
                   << "FILE_SCHEMA(('IFC4X3'));\n"
                   << "ENDSEC;\n"
                   << "DATA;\n"
                   << "#1=IFCPROJECT('0123456789012345',$,'Large Project',$,$,$,$,$,#2);\n"
                   << "#2=IFCGEOMETRICREPRESENTATIONCONTEXT($,'Model',3,1.0E-05,#3,$);\n"
                   << "#3=IFCAXIS2PLACEMENT3D(#4,$,$);\n"
                   << "#4=IFCCARTESIANPOINT((0.,0.,0.));\n";
        
        // Generiere 1000 Wände
        int entity_id = 5;
        for (int i = 0; i < 1000; i++) {
            large_file << "#" << entity_id++ << "=IFCWALL('" 
                      << std::to_string(i) << "234567890123456',$,'Wall " 
                      << i << "',$,$,#" << entity_id << ",$,$,$);\n";
            large_file << "#" << entity_id++ << "=IFCLOCALPLACEMENT($,#" 
                      << entity_id << ");\n";
            large_file << "#" << entity_id++ << "=IFCAXIS2PLACEMENT3D(#" 
                      << entity_id << ",$,$);\n";
            large_file << "#" << entity_id++ << "=IFCCARTESIANPOINT((" 
                      << i * 1000 << ".,0.,0.));\n";
        }
        
        large_file << "ENDSEC;\n"
                   << "END-ISO-10303-21;\n";
        large_file.close();
        
        // Messe Ladezeit
        auto start = std::chrono::high_resolution_clock::now();
        fixture.parser->loadFile("test_data/large.ifc");
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Sollte in angemessener Zeit laden (< 5 Sekunden)
        REQUIRE(duration.count() < 5000);
        
        // Verifiziere korrekte Anzahl
        auto json = fixture.parser->toJson();
        REQUIRE(json["entities"]["by_type"]["IfcWall"]["count"] == 1000);
    }
}