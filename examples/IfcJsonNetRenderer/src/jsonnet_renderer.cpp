#include "jsonnet_renderer.h"
#include <fstream>
#include <iostream>

JsonnetRenderer::JsonnetRenderer() {
#ifdef ENABLE_JSONNET
    m_vm = jsonnet_make();
    jsonnet_max_stack(m_vm, 200);
    jsonnet_gc_min_objects(m_vm, 1000);
    jsonnet_max_trace(m_vm, 20);
#endif
}

JsonnetRenderer::~JsonnetRenderer() {
#ifdef ENABLE_JSONNET
    if (m_vm) {
        jsonnet_destroy(m_vm);
    }
#endif
}

json JsonnetRenderer::renderTemplate(const std::string& template_content,
                                    const json& ifc_data,
                                    const std::unordered_map<std::string, std::string>& external_vars) {
#ifdef ENABLE_JSONNET
    try {
        setupVmWithIfcData(ifc_data);
        
        // Add external variables
        for (const auto& var : m_external_vars) {
            jsonnet_ext_var(m_vm, var.first.c_str(), var.second.c_str());
        }
        
        // Add provided external variables
        for (const auto& var : external_vars) {
            jsonnet_ext_var(m_vm, var.first.c_str(), var.second.c_str());
        }
        
        // Evaluate template
        int error;
        char* output = jsonnet_evaluate_snippet(m_vm, "template", template_content.c_str(), &error);
        
        if (error) {
            std::string error_msg = output ? output : "Unknown Jsonnet error";
            jsonnet_realloc(m_vm, output, 0);
            return handleJsonnetError(error_msg);
        }
        
        // Parse JSON output
        json result = json::parse(output);
        jsonnet_realloc(m_vm, output, 0);
        return result;
        
    } catch (const std::exception& e) {
        return handleJsonnetError("Template rendering failed: " + std::string(e.what()));
    }
#else
    // Fallback when jsonnet is not available
    json result;
    result["error"] = "Jsonnet support is disabled. Basic JSON data returned.";
    result["data"] = ifc_data;
    result["external_vars"] = external_vars;
    return result;
#endif
}

json JsonnetRenderer::renderTemplateFile(const std::string& template_file,
                                        const json& ifc_data,
                                        const std::unordered_map<std::string, std::string>& external_vars) {
    try {
        std::ifstream file(template_file);
        if (!file.is_open()) {
            return handleJsonnetError("Could not open template file: " + template_file);
        }
        
        std::string template_content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        file.close();
        
        return renderTemplate(template_content, ifc_data, external_vars);
        
    } catch (const std::exception& e) {
        return handleJsonnetError("Template file rendering failed: " + std::string(e.what()));
    }
}

void JsonnetRenderer::setupVmWithIfcData(const json& ifc_data) {
#ifdef ENABLE_JSONNET
    // Make IFC data available as external variable
    std::string ifc_data_str = ifc_data.dump();
    jsonnet_ext_var(m_vm, "ifcData", ifc_data_str.c_str());
    
    // Add helper functions as external code
    std::string ifc_code = "{ getByType: function(type) ifcData.entities[type] }";
    jsonnet_ext_code(m_vm, "ifc", ifc_code.c_str());
#endif
}

void JsonnetRenderer::setImportCallback(JsonnetImportCallback* import_callback) {
#ifdef ENABLE_JSONNET
    if (m_vm) {
        jsonnet_import_callback(m_vm, import_callback, nullptr);
    }
#endif
}

void JsonnetRenderer::addExternalVar(const std::string& key, const std::string& value) {
    m_external_vars[key] = value;
}

void JsonnetRenderer::clearExternalVars() {
    m_external_vars.clear();
}

json JsonnetRenderer::handleJsonnetError(const std::string& error_msg) {
    json error_response;
    error_response["error"] = error_msg;
    error_response["type"] = "jsonnet_error";
    return error_response;
}

std::string JsonnetRenderer::getDefaultIfcTemplate() {
    return R"jsonnet(
local ifc = import 'ifc';

{
    "metadata": {
        "generated": std.toString(std.thisFile),
        "elements_count": std.length(std.objectFields(ifcData.entities)),
        "project_info": ifcData.project
    },
    "elements": [
        {
            "id": entity.id,
            "type": entity.type,
            "name": if "Name" in entity.attributes then entity.attributes.Name else null,
            "description": if "Description" in entity.attributes then entity.attributes.Description else null,
            "properties": entity.properties,
            "geometry": if "geometry" in entity then entity.geometry else null
        }
        for entity in std.flattenArrays([
            ifcData.entities[entity_type]
            for entity_type in std.objectFields(ifcData.entities)
        ])
    ],
    "summary": {
        "by_type": {
            [entity_type]: std.length(ifcData.entities[entity_type])
            for entity_type in std.objectFields(ifcData.entities)
        }
    }
}
)jsonnet";
}

std::string JsonnetRenderer::getElementTypeTemplate(const std::string& element_type) {
    std::string template_str = R"jsonnet(
local ifc = import 'ifc';
local element_type = ')jsonnet" + element_type + R"jsonnet(';

{
    "element_type": element_type,
    "count": if element_type in ifcData.entities then std.length(ifcData.entities[element_type]) else 0,
    "elements": if element_type in ifcData.entities then [
        {
            "id": element.id,
            "name": if "Name" in element.attributes then element.attributes.Name else null,
            "description": if "Description" in element.attributes then element.attributes.Description else null,
            "properties": element.properties,
            "geometry": if "geometry" in element then {
                "vertices_count": if "vertices" in element.geometry then std.length(element.geometry.vertices) else 0,
                "faces_count": if "faces" in element.geometry then std.length(element.geometry.faces) else 0,
                "bounding_box": if "bounding_box" in element.geometry then element.geometry.bounding_box else null
            } else null,
            "relationships": if "relationships" in element then element.relationships else []
        }
        for element in ifcData.entities[element_type]
    ] else [],
    "metadata": {
        "query_type": "element_type_filter",
        "filter": element_type,
        "generated_at": std.toString(std.thisFile)
    }
}
)jsonnet";
    
    return template_str;
}