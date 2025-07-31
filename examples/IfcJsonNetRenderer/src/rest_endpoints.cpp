#include "rest_endpoints.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

using json = nlohmann::json;

RestEndpoints::RestEndpoints() {
    m_ifc_parser = std::make_unique<IfcParser>();
    m_jsonnet_renderer = std::make_unique<JsonnetRenderer>();
}

RestEndpoints::~RestEndpoints() {
    // Cleanup handled by unique_ptr
}

void RestEndpoints::setupEndpoints(crow::SimpleApp& app) {
    // CORS preflight handler
    CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)
    ([this](const crow::request& req, crow::response& res, const std::string& path) {
        res.code = 200;
        addCorsHeaders(res);
        res.end();
    });

    // Load IFC file
    CROW_ROUTE(app, "/api/ifc/load").methods("POST"_method)
    ([this](const crow::request& req) {
        return handleLoadIfc(req);
    });

    // Get entities by type
    CROW_ROUTE(app, "/api/ifc/entities/<string>")
    ([this](const crow::request& req, const std::string& type) {
        return handleGetEntitiesByType(req, type);
    });

    // Get entity geometry
    CROW_ROUTE(app, "/api/ifc/entity/<string>/geometry")
    ([this](const crow::request& req, const std::string& id) {
        return handleGetEntityGeometry(req, id);
    });

    // Get building hierarchy
    CROW_ROUTE(app, "/api/ifc/hierarchy")
    ([this](const crow::request& req) {
        return handleGetHierarchy(req);
    });

    // Render template
    CROW_ROUTE(app, "/api/render/template").methods("POST"_method)
    ([this](const crow::request& req) {
        return handleRenderTemplate(req);
    });

    // Render template file
    CROW_ROUTE(app, "/api/render/template/file").methods("POST"_method)
    ([this](const crow::request& req) {
        return handleRenderTemplateFile(req);
    });

    // Get default template
    CROW_ROUTE(app, "/api/render/templates/default")
    ([this](const crow::request& req) {
        return handleGetDefaultTemplate(req);
    });

    // Get element template
    CROW_ROUTE(app, "/api/render/templates/element/<string>")
    ([this](const crow::request& req, const std::string& type) {
        return handleGetElementTemplate(req, type);
    });

    // Get server status
    CROW_ROUTE(app, "/api/status")
    ([this](const crow::request& req) {
        return handleGetStatus(req);
    });

    // Root endpoint
    CROW_ROUTE(app, "/")
    ([](const crow::request& req, crow::response& res) {
        json info;
        info["name"] = "IFC JSON Renderer API";
        info["version"] = "1.0.0";
        info["description"] = "REST API for converting IFC files to JSON and rendering with Jsonnet templates";
        info["endpoints"] = {
            "POST /api/ifc/load",
            "GET /api/ifc/entities/{type}",
            "GET /api/ifc/entity/{id}/geometry",
            "GET /api/ifc/hierarchy",
            "POST /api/render/template",
            "POST /api/render/template/file",
            "GET /api/render/templates/default",
            "GET /api/render/templates/element/{type}",
            "GET /api/status"
        };
        
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(info.dump(2));
        res.end();
    });
}

crow::response RestEndpoints::handleLoadIfc(const crow::request& req) {
    try {
        json request_body = json::parse(req.body);
        
        if (!request_body.contains("filename")) {
            return createErrorResponse("Missing 'filename' parameter");
        }
        
        std::string filename = request_body["filename"];
        
        // Load IFC file
        json ifc_data = m_ifc_parser->loadIfcFile(filename);
        
        if (ifc_data.contains("error")) {
            return createErrorResponse(ifc_data["error"], 500);
        }
        
        // Cache the loaded data
        m_ifc_cache[filename] = ifc_data;
        
        json response;
        response["message"] = "IFC file loaded successfully";
        response["filename"] = filename;
        response["data"] = ifc_data;
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Invalid JSON in request body: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetEntitiesByType(const crow::request& req, const std::string& type) {
    try {
        json entities = m_ifc_parser->getEntitiesByType(type);
        
        json response;
        response["entityType"] = type;
        response["entities"] = entities;
        response["count"] = entities.size();
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get entities: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetEntityGeometry(const crow::request& req, const std::string& id) {
    try {
        json geometry = m_ifc_parser->getEntityGeometry(id);
        
        json response;
        response["entityId"] = id;
        response["geometry"] = geometry;
        response["hasGeometry"] = !geometry.is_null();
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get entity geometry: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetHierarchy(const crow::request& req) {
    try {
        json hierarchy = m_ifc_parser->getBuildingHierarchy();
        
        json response;
        response["hierarchy"] = hierarchy;
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get building hierarchy: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleRenderTemplate(const crow::request& req) {
    try {
        json request_body = json::parse(req.body);
        
        if (!request_body.contains("template")) {
            return createErrorResponse("Missing 'template' parameter");
        }
        
        std::string template_content = request_body["template"];
        
        // Get IFC data (use the most recently loaded file or empty data)
        json ifc_data;
        if (!m_ifc_cache.empty()) {
            ifc_data = m_ifc_cache.rbegin()->second;
        }
        
        // Get external variables
        std::map<std::string, std::string> external_vars;
        if (request_body.contains("variables")) {
            for (auto& [key, value] : request_body["variables"].items()) {
                external_vars[key] = value.get<std::string>();
            }
        }
        
        // Add timestamp
        external_vars["timestamp"] = getCurrentTimestamp();
        
        // Render template
        json result = m_jsonnet_renderer->renderTemplate(template_content, ifc_data, external_vars);
        
        return createJsonResponse(result);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Template rendering failed: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleRenderTemplateFile(const crow::request& req) {
    try {
        json request_body = json::parse(req.body);
        
        if (!request_body.contains("templateFile")) {
            return createErrorResponse("Missing 'templateFile' parameter");
        }
        
        std::string template_file = request_body["templateFile"];
        
        // Get IFC data
        json ifc_data;
        if (!m_ifc_cache.empty()) {
            ifc_data = m_ifc_cache.rbegin()->second;
        }
        
        // Get external variables
        std::map<std::string, std::string> external_vars;
        if (request_body.contains("variables")) {
            for (auto& [key, value] : request_body["variables"].items()) {
                external_vars[key] = value.get<std::string>();
            }
        }
        
        // Add timestamp
        external_vars["timestamp"] = getCurrentTimestamp();
        
        // Render template file
        json result = m_jsonnet_renderer->renderTemplateFile(template_file, ifc_data, external_vars);
        
        return createJsonResponse(result);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Template file rendering failed: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetDefaultTemplate(const crow::request& req) {
    try {
        std::string template_content = JsonnetRenderer::getDefaultIfcTemplate();
        
        json response;
        response["template"] = template_content;
        response["description"] = "Default IFC template for basic visualization";
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get default template: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetElementTemplate(const crow::request& req, const std::string& type) {
    try {
        std::string template_content = JsonnetRenderer::getElementTypeTemplate(type);
        
        json response;
        response["template"] = template_content;
        response["elementType"] = type;
        response["description"] = "Template for " + type + " elements";
        response["timestamp"] = getCurrentTimestamp();
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get element template: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::handleGetStatus(const crow::request& req) {
    try {
        json response;
        response["status"] = "running";
        response["loadedFiles"] = json::array();
        
        for (const auto& [filename, data] : m_ifc_cache) {
            json file_info;
            file_info["filename"] = filename;
            file_info["entityCount"] = data.contains("entities") ? data["entities"].size() : 0;
            response["loadedFiles"].push_back(file_info);
        }
        
        response["timestamp"] = getCurrentTimestamp();
        response["version"] = "1.0.0";
        
        return createJsonResponse(response);
        
    } catch (const std::exception& e) {
        return createErrorResponse("Failed to get status: " + std::string(e.what()));
    }
}

crow::response RestEndpoints::createErrorResponse(const std::string& message, int status_code) {
    json error_json;
    error_json["error"] = message;
    error_json["timestamp"] = getCurrentTimestamp();
    
    crow::response res(status_code);
    addCorsHeaders(res);
    res.set_header("Content-Type", "application/json");
    res.write(error_json.dump(2));
    return res;
}

crow::response RestEndpoints::createJsonResponse(const json& json_data, int status_code) {
    crow::response res(status_code);
    addCorsHeaders(res);
    res.set_header("Content-Type", "application/json");
    res.write(json_data.dump(2));
    return res;
}

void RestEndpoints::addCorsHeaders(crow::response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

std::string RestEndpoints::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}