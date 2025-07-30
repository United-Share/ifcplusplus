#include <iostream>
#include <string>
#include <signal.h>
#include <crow.h>
#include "rest_endpoints.h"

// Global server instance for signal handling
crow::SimpleApp* g_app = nullptr;
RestEndpoints* g_endpoints = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_app) {
        g_app->stop();
    }
    exit(0);
}

void printBanner() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                  IFC JSON Renderer Server                   ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  A REST API server for converting IFC files to JSON and     ║" << std::endl;
    std::cout << "║  rendering them with Jsonnet templates on-demand.           ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  Features:                                                   ║" << std::endl;
    std::cout << "║  • Load and parse IFC files to JSON                         ║" << std::endl;
    std::cout << "║  • Extract geometry and building hierarchy                   ║" << std::endl;
    std::cout << "║  • Query entities by type                                    ║" << std::endl;
    std::cout << "║  • Render Jsonnet templates with IFC data                   ║" << std::endl;
    std::cout << "║  • RESTful API with CORS support                            ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
}

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p, --port PORT    Set server port (default: 8080)" << std::endl;
    std::cout << "  -h, --help         Show this help message" << std::endl;
    std::cout << "  -v, --version      Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Start server on port 8080" << std::endl;
    std::cout << "  " << program_name << " -p 3000            # Start server on port 3000" << std::endl;
    std::cout << std::endl;
}

void printEndpoints(int port) {
    std::cout << "Available REST API endpoints:" << std::endl;
    std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│ GET  /                                    - API information  │" << std::endl;
    std::cout << "│ GET  /api/status                          - Server status   │" << std::endl;
    std::cout << "│ POST /api/ifc/load                        - Load IFC file   │" << std::endl;
    std::cout << "│ GET  /api/ifc/entities/{type}             - Get entities    │" << std::endl;
    std::cout << "│ GET  /api/ifc/entity/{id}/geometry        - Get geometry    │" << std::endl;
    std::cout << "│ GET  /api/ifc/hierarchy                   - Get hierarchy   │" << std::endl;
    std::cout << "│ POST /api/render/template                 - Render template │" << std::endl;
    std::cout << "│ POST /api/render/template/file            - Render file     │" << std::endl;
    std::cout << "│ GET  /api/render/templates/default        - Get default tmpl│" << std::endl;
    std::cout << "│ GET  /api/render/templates/element/{type} - Get element tmpl│" << std::endl;
    std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << std::endl;
    std::cout << "Server URL: http://localhost:" << port << std::endl;
    std::cout << "Try: curl http://localhost:" << port << "/api/status" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Parse command line arguments
    int port = 8080;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            std::cout << "IFC JSON Renderer Server v1.0.0" << std::endl;
            std::cout << "Built with:" << std::endl;
            std::cout << "  • IFC++ (IfcPlusPlus)" << std::endl;
            std::cout << "  • Crow C++ framework" << std::endl;
            std::cout << "  • Jsonnet library" << std::endl;
            std::cout << "  • nlohmann/json" << std::endl;
            return 0;
        }
        else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
                if (port < 1 || port > 65535) {
                    std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port number" << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown argument '" << arg << "'" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Create Crow application
        crow::SimpleApp app;
        g_app = &app;
        
        // Setup CORS middleware
        app.use_compression(crow::compression::algorithm::GZIP);
        
        // Create REST endpoints handler
        RestEndpoints endpoints;
        g_endpoints = &endpoints;
        
        // Setup all endpoints
        endpoints.setupEndpoints(app);
        
        std::cout << "Initializing IFC JSON Renderer Server..." << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << std::endl;
        
        printEndpoints(port);
        
        std::cout << "Starting server..." << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        
        // Start the server
        app.port(port).multithreaded().run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Server stopped." << std::endl;
    return 0;
}