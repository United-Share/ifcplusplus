#pragma once

#include <crow.h>
#include <memory>
#include <map>
#include <string>
#include "ifc_parser.h"
#include "jsonnet_renderer.h"

/**
 * REST API endpoints for IFC JSON rendering
 */
class RestEndpoints {
public:
    RestEndpoints();
    ~RestEndpoints();

    /**
     * Setup all REST endpoints with the Crow app
     * @param app The Crow application
     */
    void setupEndpoints(crow::SimpleApp& app);

    /**
     * Start the server
     * @param port Port number to listen on
     */
    void startServer(int port = 8080);

private:
    std::unique_ptr<IfcParser> m_ifc_parser;
    std::unique_ptr<JsonnetRenderer> m_jsonnet_renderer;
    
    // Cache for loaded IFC files
    std::map<std::string, nlohmann::json> m_ifc_cache;

    /**
     * POST /api/ifc/load
     * Load an IFC file and return JSON representation
     */
    crow::response handleLoadIfc(const crow::request& req);

    /**
     * GET /api/ifc/entities/{type}
     * Get all entities of a specific type
     */
    crow::response handleGetEntitiesByType(const crow::request& req, const std::string& type);

    /**
     * GET /api/ifc/entity/{id}/geometry
     * Get geometry data for a specific entity
     */
    crow::response handleGetEntityGeometry(const crow::request& req, const std::string& id);

    /**
     * GET /api/ifc/hierarchy
     * Get building hierarchy
     */
    crow::response handleGetHierarchy(const crow::request& req);

    /**
     * POST /api/render/template
     * Render a Jsonnet template with current IFC data
     */
    crow::response handleRenderTemplate(const crow::request& req);

    /**
     * POST /api/render/template/file
     * Load and render a Jsonnet template file
     */
    crow::response handleRenderTemplateFile(const crow::request& req);

    /**
     * GET /api/render/templates/default
     * Get the default IFC template
     */
    crow::response handleGetDefaultTemplate(const crow::request& req);

    /**
     * GET /api/render/templates/element/{type}
     * Get template for specific element type
     */
    crow::response handleGetElementTemplate(const crow::request& req, const std::string& type);

    /**
     * GET /api/status
     * Get server status and loaded IFC files
     */
    crow::response handleGetStatus(const crow::request& req);

    /**
     * Helper method to create error response
     * @param message Error message
     * @param status_code HTTP status code
     * @return Crow response
     */
    crow::response createErrorResponse(const std::string& message, int status_code = 400);

    /**
     * Helper method to create JSON response
     * @param json_data JSON data
     * @param status_code HTTP status code
     * @return Crow response
     */
    crow::response createJsonResponse(const nlohmann::json& json_data, int status_code = 200);

    /**
     * Add CORS headers to response
     * @param res Response to add headers to
     */
    void addCorsHeaders(crow::response& res);

    /**
     * Get current timestamp as string
     * @return Timestamp string
     */
    std::string getCurrentTimestamp();
};