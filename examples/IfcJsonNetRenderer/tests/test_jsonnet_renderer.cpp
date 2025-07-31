// test_jsonnet_renderer.cpp - Tests für Jsonnet Renderer
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "jsonnet_renderer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using Catch::Matchers::Contains;

class JsonnetRendererTestFixture {
protected:
    std::unique_ptr<JsonnetRenderer> renderer;
    json sample_ifc_data;
    
    JsonnetRendererTestFixture() {
        renderer = std::make_unique<JsonnetRenderer>();
        
        // Beispiel IFC-Daten für Tests
        sample_ifc_data = {
            {"project", {
                {"name", "Test Project"},
                {"guid", "1234567890123456"}
            }},
            {"entities", {
                {"total_count", 10},
                {"by_type", {
                    {"IfcWall", {
                        {"count", 5},
                        {"items", json::array({
                            {{"guid", "wall1"}, {"name", "Wall 1"}},
                            {{"guid", "wall2"}, {"name", "Wall 2"}}
                        })}
                    }},
                    {"IfcSlab", {
                        {"count", 3}
                    }}
                }}
            }}
        };
    }
};

TEST_CASE("JsonnetRenderer Basic Functionality", "[jsonnet_renderer]") {
    JsonnetRendererTestFixture fixture;
    
    SECTION("Renderer can be instantiated") {
        REQUIRE(fixture.renderer != nullptr);
    }
    
    SECTION("Render simple template") {
        std::string template_content = R"({
            project_name: std.extVar("ifc_data").project.name,
            wall_count: std.extVar("ifc_data").entities.by_type.IfcWall.count
        })";
        
        auto result = fixture.renderer->renderTemplate(
            template_content, 
            fixture.sample_ifc_data
        );
        
        REQUIRE(result.is_object());
        REQUIRE(result["project_name"] == "Test Project");
        REQUIRE(result["wall_count"] == 5);
    }
}

TEST_CASE("JsonnetRenderer Advanced Templates", "[jsonnet_renderer][advanced]") {
    JsonnetRendererTestFixture fixture;
    
    SECTION("Template with functions and loops") {
        std::string template_content = R"({
            local ifc = std.extVar("ifc_data"),
            
            walls: [
                {
                    id: wall.guid,
                    name: wall.name,
                    uppercase_name: std.asciiUpper(wall.name)
                }
                for wall in ifc.entities.by_type.IfcWall.items
            ],
            
            summary: {
                total_walls: std.length(self.walls),
                has_walls: std.length(self.walls) > 0
            }
        })";
        
        auto result = fixture.renderer->renderTemplate(
            template_content,
            fixture.sample_ifc_data
        );
        
        REQUIRE(result["walls"].is_array());
        REQUIRE(result["walls"].size() == 2);
        REQUIRE(result["walls"][0]["uppercase_name"] == "WALL 1");
        REQUIRE(result["summary"]["total_walls"] == 2);
        REQUIRE(result["summary"]["has_walls"] == true);
    }
    
    SECTION("Template with external variables") {
        std::string template_content = R"({
            project: std.extVar("ifc_data").project.name,
            user: std.extVar("user"),
            timestamp: std.extVar("timestamp")
        })";
        
        std::map<std::string, std::string> vars = {
            {"user", "test_user"},
            {"timestamp", "2024-01-01T00:00:00Z"}
        };
        
        auto result = fixture.renderer->renderTemplate(
            template_content,
            fixture.sample_ifc_data,
            vars
        );
        
        REQUIRE(result["project"] == "Test Project");
        REQUIRE(result["user"] == "test_user");
        REQUIRE(result["timestamp"] == "2024-01-01T00:00:00Z");
    }
}

TEST_CASE("JsonnetRenderer Error Handling", "[jsonnet_renderer][error]") {
    JsonnetRendererTestFixture fixture;
    
    SECTION("Invalid template syntax") {
        std::string invalid_template = R"({
            invalid: syntax here
        })";
        
        auto result = fixture.renderer->renderTemplate(
            invalid_template,
            fixture.sample_ifc_data
        );
        
        REQUIRE(result.is_object());
        REQUIRE(result.contains("error"));
        REQUIRE(result["error"].is_string());
    }
    
    SECTION("Missing external variable") {
        std::string template_content = R"({
            missing: std.extVar("non_existent_var")
        })";
        
        auto result = fixture.renderer->renderTemplate(
            template_content,
            fixture.sample_ifc_data
        );
        
        REQUIRE(result.contains("error"));
        REQUIRE_THAT(result["error"].get<std::string>(), 
                     Contains("non_existent_var"));
    }
}

TEST_CASE("JsonnetRenderer File Operations", "[jsonnet_renderer][file]") {
    JsonnetRendererTestFixture fixture;
    
    SECTION("Load template from file") {
        // Erstelle temporäre Template-Datei
        std::string temp_file = "test_template.jsonnet";
        std::ofstream file(temp_file);
        file << R"({
            project_info: {
                name: std.extVar("ifc_data").project.name,
                guid: std.extVar("ifc_data").project.guid
            }
        })";
        file.close();
        
        auto result = fixture.renderer->renderTemplateFile(
            temp_file,
            fixture.sample_ifc_data
        );
        
        REQUIRE(result["project_info"]["name"] == "Test Project");
        REQUIRE(result["project_info"]["guid"] == "1234567890123456");
        
        // Aufräumen
        std::remove(temp_file.c_str());
    }
}

TEST_CASE("JsonnetRenderer Default Templates", "[jsonnet_renderer][defaults]") {
    JsonnetRendererTestFixture fixture;
    
    SECTION("Get default IFC template") {
        auto default_template = JsonnetRenderer::getDefaultIfcTemplate();
        
        REQUIRE(!default_template.empty());
        REQUIRE_THAT(default_template, Contains("ifc_data"));
    }
    
    SECTION("Get element-specific template") {
        auto wall_template = JsonnetRenderer::getElementTypeTemplate("IfcWall");
        
        REQUIRE(!wall_template.empty());
        REQUIRE_THAT(wall_template, Contains("IfcWall"));
    }
}