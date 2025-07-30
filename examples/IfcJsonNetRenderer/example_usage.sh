#!/bin/bash

# IFC JSON Renderer - Example Usage Script
# This script demonstrates how to use the REST API

SERVER_URL="http://localhost:8080"
IFC_FILE="../LoadFileExample/example.ifc"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║           IFC JSON Renderer - API Usage Examples           ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Function to make HTTP requests with pretty output
make_request() {
    local method="$1"
    local endpoint="$2"
    local data="$3"
    local description="$4"
    
    echo "┌─────────────────────────────────────────────────────────────┐"
    echo "│ $description"
    echo "└─────────────────────────────────────────────────────────────┘"
    echo "Request: $method $SERVER_URL$endpoint"
    
    if [ -n "$data" ]; then
        echo "Data: $data"
        echo ""
        curl -s -X "$method" "$SERVER_URL$endpoint" \
             -H "Content-Type: application/json" \
             -d "$data" | python3 -m json.tool 2>/dev/null || echo "Response: (not valid JSON)"
    else
        echo ""
        curl -s -X "$method" "$SERVER_URL$endpoint" | python3 -m json.tool 2>/dev/null || echo "Response: (not valid JSON)"
    fi
    
    echo ""
    echo "Press Enter to continue..."
    read -r
}

# Check if server is running
echo "Checking if server is running..."
if ! curl -s "$SERVER_URL/api/status" > /dev/null; then
    echo "❌ Server is not running at $SERVER_URL"
    echo "Please start the server first: ./IfcJsonNetRenderer"
    exit 1
fi
echo "✅ Server is running"
echo ""

# 1. Get API information
make_request "GET" "/" "" "Get API Information"

# 2. Get server status
make_request "GET" "/api/status" "" "Get Server Status"

# 3. Load an IFC file (if it exists)
if [ -f "$IFC_FILE" ]; then
    IFC_FILE_ABS=$(realpath "$IFC_FILE")
    make_request "POST" "/api/ifc/load" "{\"filename\": \"$IFC_FILE_ABS\"}" "Load IFC File"
else
    echo "❌ IFC file not found at $IFC_FILE"
    echo "Skipping IFC file loading examples..."
    echo ""
fi

# 4. Get building hierarchy
make_request "GET" "/api/ifc/hierarchy" "" "Get Building Hierarchy"

# 5. Get walls
make_request "GET" "/api/ifc/entities/IfcWall" "" "Get All Walls"

# 6. Get spaces
make_request "GET" "/api/ifc/entities/IfcSpace" "" "Get All Spaces"

# 7. Get default template
make_request "GET" "/api/render/templates/default" "" "Get Default Jsonnet Template"

# 8. Get element template for walls
make_request "GET" "/api/render/templates/element/IfcWall" "" "Get Wall Element Template"

# 9. Render simple template
SIMPLE_TEMPLATE='local ifc = std.extVar("ifc"); { totalEntities: std.length(ifc.entities), timestamp: std.extVar("timestamp") }'
make_request "POST" "/api/render/template" "{\"template\": \"$SIMPLE_TEMPLATE\"}" "Render Simple Template"

# 10. Render default template
DEFAULT_TEMPLATE_DATA='{"template": "local ifc = std.extVar(\"ifc\"); { project: { name: if std.objectHas(ifc.project, \"name\") then ifc.project.name else \"Unknown Project\" }, summary: { totalEntities: std.length(ifc.entities), entityTypes: std.set([entity.type for entity in ifc.entities]) }, rendered: std.extVar(\"timestamp\") }"}'
make_request "POST" "/api/render/template" "$DEFAULT_TEMPLATE_DATA" "Render Enhanced Template"

# 11. Render wall analysis template (if file exists)
if [ -f "templates/wall-analysis.jsonnet" ]; then
    make_request "POST" "/api/render/template/file" '{"templateFile": "templates/wall-analysis.jsonnet"}' "Render Wall Analysis Template"
else
    echo "Templates not found. You can copy them to the working directory."
fi

# 12. Render building summary template (if file exists)
if [ -f "templates/building-summary.jsonnet" ]; then
    make_request "POST" "/api/render/template/file" '{"templateFile": "templates/building-summary.jsonnet"}' "Render Building Summary Template"
fi

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                      Examples Complete                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "You can now experiment with:"
echo "• Loading different IFC files"
echo "• Creating custom Jsonnet templates"
echo "• Querying specific entity types"
echo "• Analyzing geometry data"
echo ""
echo "For more information, see the README.md file."