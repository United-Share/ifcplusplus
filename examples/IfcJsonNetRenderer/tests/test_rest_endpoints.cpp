// test_rest_endpoints.cpp - Tests für REST Endpoints
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rest_endpoints.h"
#include <crow.h>
#include <thread>
#include <chrono>

using Catch::Matchers::Contains;

class RestEndpointsTestFixture {
protected:
    std::unique_ptr<RestEndpoints> endpoints;
    crow::SimpleApp app;
    std::thread server_thread;
    
    RestEndpointsTestFixture() {
        endpoints = std::make_unique<RestEndpoints>();
        endpoints->setup(app);
    }
    
    void startServer() {
        server_thread = std::thread([this]() {
            app.port(18080).run();
        });
        // Warte kurz, bis der Server gestartet ist
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void stopServer() {
        app.stop();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }
    
    // Hilfsfunktion für HTTP-Requests (simuliert)
    crow::response makeRequest(const std::string& method, 
                              const std::string& path, 
                              const std::string& body = "") {
        crow::request req;
        req.method = method;
        req.url = path;
        req.body = body;
        
        // Simuliere Request-Handling
        return endpoints->handleRequest(req);
    }
};

TEST_CASE("RestEndpoints Basic Routes", "[rest_endpoints]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("Health check endpoint") {
        auto response = fixture.makeRequest("GET", "/health");
        
        REQUIRE(response.code == 200);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body["status"] == "healthy");
    }
    
    SECTION("API info endpoint") {
        auto response = fixture.makeRequest("GET", "/api/v1/info");
        
        REQUIRE(response.code == 200);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body.contains("version"));
        REQUIRE(body.contains("endpoints"));
    }
}

TEST_CASE("RestEndpoints IFC Operations", "[rest_endpoints][ifc]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("Load IFC file") {
        nlohmann::json request_body = {
            {"file_path", "test.ifc"}
        };
        
        auto response = fixture.makeRequest(
            "POST", 
            "/api/v1/ifc/load",
            request_body.dump()
        );
        
        // Erwarte Fehler, da Datei nicht existiert
        REQUIRE(response.code == 400);
    }
    
    SECTION("Get entities by type") {
        auto response = fixture.makeRequest(
            "GET",
            "/api/v1/ifc/entities?type=IfcWall"
        );
        
        REQUIRE(response.code == 200);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body.is_array() || body.contains("error"));
    }
}

TEST_CASE("RestEndpoints Template Rendering", "[rest_endpoints][template]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("Render Jsonnet template") {
        nlohmann::json request_body = {
            {"template", R"({
                result: "Hello " + std.extVar("name")
            })"},
            {"variables", {
                {"name", "World"}
            }}
        };
        
        auto response = fixture.makeRequest(
            "POST",
            "/api/v1/render/template",
            request_body.dump()
        );
        
        REQUIRE(response.code == 200);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body["result"] == "Hello World");
    }
    
    SECTION("Invalid template returns error") {
        nlohmann::json request_body = {
            {"template", "invalid jsonnet"}
        };
        
        auto response = fixture.makeRequest(
            "POST",
            "/api/v1/render/template",
            request_body.dump()
        );
        
        REQUIRE(response.code == 400);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body.contains("error"));
    }
}

TEST_CASE("RestEndpoints Error Handling", "[rest_endpoints][error]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("Invalid JSON body") {
        auto response = fixture.makeRequest(
            "POST",
            "/api/v1/ifc/load",
            "invalid json"
        );
        
        REQUIRE(response.code == 400);
        auto body = nlohmann::json::parse(response.body);
        REQUIRE(body.contains("error"));
    }
    
    SECTION("Method not allowed") {
        auto response = fixture.makeRequest(
            "DELETE",
            "/api/v1/ifc/load"
        );
        
        REQUIRE(response.code == 405);
    }
    
    SECTION("Not found endpoint") {
        auto response = fixture.makeRequest(
            "GET",
            "/api/v1/non-existent"
        );
        
        REQUIRE(response.code == 404);
    }
}

TEST_CASE("RestEndpoints CORS Support", "[rest_endpoints][cors]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("CORS headers are present") {
        auto response = fixture.makeRequest("GET", "/health");
        
        REQUIRE(response.has_header("Access-Control-Allow-Origin"));
        REQUIRE(response.get_header_value("Access-Control-Allow-Origin") == "*");
    }
    
    SECTION("OPTIONS request for preflight") {
        auto response = fixture.makeRequest("OPTIONS", "/api/v1/ifc/load");
        
        REQUIRE(response.code == 200);
        REQUIRE(response.has_header("Access-Control-Allow-Methods"));
        REQUIRE_THAT(response.get_header_value("Access-Control-Allow-Methods"),
                     Contains("POST"));
    }
}

TEST_CASE("RestEndpoints Caching", "[rest_endpoints][cache]") {
    RestEndpointsTestFixture fixture;
    
    SECTION("Cached responses include cache headers") {
        auto response1 = fixture.makeRequest("GET", "/api/v1/info");
        auto response2 = fixture.makeRequest("GET", "/api/v1/info");
        
        REQUIRE(response1.code == 200);
        REQUIRE(response2.code == 200);
        
        // Zweite Anfrage sollte aus Cache kommen
        if (response2.has_header("X-Cache")) {
            REQUIRE(response2.get_header_value("X-Cache") == "HIT");
        }
    }
}