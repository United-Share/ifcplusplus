#include "jsonnet_renderer.h"
#include <fstream>
#include <sstream>
#include <iostream>

JsonnetRenderer::JsonnetRenderer() {
    m_vm = jsonnet_make();
    jsonnet_max_stack(m_vm, 200);
    jsonnet_gc_min_objects(m_vm, 1000);
    jsonnet_max_trace(m_vm, 20);
}

JsonnetRenderer::~JsonnetRenderer() {
    if (m_vm) {
        jsonnet_destroy(m_vm);
    }
}

json JsonnetRenderer::renderTemplate(const std::string& template_content, 
                                    const json& ifc_data,
                                    const std::map<std::string, std::string>& external_vars) {
    try {
        // Setup VM with IFC data
        setupVmWithIfcData(ifc_data);
        
        // Add external variables
        for (const auto& var : external_vars) {
            jsonnet_ext_var(m_vm, var.first.c_str(), var.second.c_str());
        }
        
        // Add stored external variables
        for (const auto& var : m_external_vars) {
            jsonnet_ext_var(m_vm, var.first.c_str(), var.second.c_str());
        }
        
        // Evaluate the template
        int error;
        char* output = jsonnet_evaluate_snippet(m_vm, "template", template_content.c_str(), &error);
        
        if (error) {
            std::string error_msg = output ? output : "Unknown Jsonnet error";
            jsonnet_realloc(m_vm, output, 0);
            return handleJsonnetError(error_msg);
        }
        
        // Parse the output JSON
        std::string result_str(output);
        jsonnet_realloc(m_vm, output, 0);
        
        return json::parse(result_str);
        
    } catch (const std::exception& e) {
        return handleJsonnetError("Template rendering failed: " + std::string(e.what()));
    }
}

json JsonnetRenderer::renderTemplateFile(const std::string& template_file,
                                        const json& ifc_data,
                                        const std::map<std::string, std::string>& external_vars) {
    try {
        // Read template file
        std::ifstream file(template_file);
        if (!file.is_open()) {
            return handleJsonnetError("Could not open template file: " + template_file);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string template_content = buffer.str();
        
        return renderTemplate(template_content, ifc_data, external_vars);
        
    } catch (const std::exception& e) {
        return handleJsonnetError("Template file rendering failed: " + std::string(e.what()));
    }
}

void JsonnetRenderer::setupVmWithIfcData(const json& ifc_data) {
    // Convert JSON to string and set as external variable
    std::string ifc_data_str = ifc_data.dump();
    jsonnet_ext_var(m_vm, "ifcData", ifc_data_str.c_str());
    
    // Also provide parsed JSON as external code
    std::string ifc_code = "std.parseJson(std.extVar('ifcData'))";
    jsonnet_ext_code(m_vm, "ifc", ifc_code.c_str());
}

void JsonnetRenderer::setImportCallback(JsonnetImportCallback* import_callback) {
    if (m_vm && import_callback) {
        jsonnet_import_callback(m_vm, import_callback, nullptr);
    }
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
local ifc = std.extVar('ifc');

{
  // Basic project information
  project: {
    name: if std.objectHas(ifc.project, 'name') then ifc.project.name else 'Unknown Project',
    description: if std.objectHas(ifc.project, 'description') then ifc.project.description else '',
    globalId: if std.objectHas(ifc.project, 'globalId') then ifc.project.globalId else '',
  },
  
  // Summary statistics
  summary: {
    totalEntities: std.length(ifc.entities),
    entityTypes: std.set([entity.type for entity in ifc.entities]),
    entitiesWithGeometry: std.length([entity for entity in ifc.entities if std.objectHas(entity, 'geometry')]),
  },
  
  // Entities grouped by type
  entitiesByType: {
    [entityType]: [
      {
        id: entity.id,
        globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
        name: if std.objectHas(entity, 'name') then entity.name else '',
        description: if std.objectHas(entity, 'description') then entity.description else '',
        hasGeometry: std.objectHas(entity, 'geometry'),
      }
      for entity in ifc.entities
      if entity.type == entityType
    ]
    for entityType in std.set([entity.type for entity in ifc.entities])
  },
  
  // Render timestamp
  rendered: std.extVar('timestamp'),
}
)jsonnet";
}

std::string JsonnetRenderer::getElementTypeTemplate(const std::string& element_type) {
    std::string template_str = R"jsonnet(
local ifc = std.extVar('ifc');
local elementType = std.extVar('elementType');

{
  elementType: elementType,
  elements: [
    {
      id: entity.id,
      globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
      name: if std.objectHas(entity, 'name') then entity.name else '',
      description: if std.objectHas(entity, 'description') then entity.description else '',
      properties: entity.properties,
      geometry: if std.objectHas(entity, 'geometry') then {
        hasGeometry: true,
        meshCount: std.length(entity.geometry.meshes),
        boundingBox: {
          // Calculate bounding box from vertices
          min: std.foldl(
            function(acc, mesh) 
              std.foldl(
                function(acc2, vertex) [
                  std.min(acc2[0], vertex[0]),
                  std.min(acc2[1], vertex[1]),
                  std.min(acc2[2], vertex[2])
                ],
                mesh.vertices,
                acc
              ),
            entity.geometry.meshes,
            [1e10, 1e10, 1e10]
          ),
          max: std.foldl(
            function(acc, mesh) 
              std.foldl(
                function(acc2, vertex) [
                  std.max(acc2[0], vertex[0]),
                  std.max(acc2[1], vertex[1]),
                  std.max(acc2[2], vertex[2])
                ],
                mesh.vertices,
                acc
              ),
            entity.geometry.meshes,
            [-1e10, -1e10, -1e10]
          ),
        }
      } else {
        hasGeometry: false
      }
    }
    for entity in ifc.entities
    if entity.type == elementType
  ],
  count: std.length([entity for entity in ifc.entities if entity.type == elementType]),
  rendered: std.extVar('timestamp'),
}
)jsonnet";
    
    return template_str;
}