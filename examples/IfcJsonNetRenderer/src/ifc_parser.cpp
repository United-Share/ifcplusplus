#include "ifc_parser.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <ifcpp/IFC4X3/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4X3/include/IfcLabel.h>
#include <ifcpp/IFC4X3/include/IfcText.h>
#include <ifcpp/IFC4X3/include/IfcRelAggregates.h>
#include <ifcpp/IFC4X3/include/IfcRelContainedInSpatialStructure.h>
#include <ifcpp/IFC4X3/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4X3/include/IfcProduct.h>
#include <ifcpp/model/BuildingObject.h>

using namespace IFC4X3;

IfcParser::IfcParser() {
    m_ifc_model = std::make_shared<BuildingModel>();
    m_step_reader = std::make_shared<ReaderSTEP>();
}

IfcParser::~IfcParser() {
    // Cleanup handled by smart pointers
}

json IfcParser::loadIfcFile(const std::string& filename) {
    try {
        // Load the IFC file
        m_step_reader->loadModelFromFile(filename, m_ifc_model);
        
        // Initialize geometry converter
        std::shared_ptr<GeometrySettings> geom_settings = std::make_shared<GeometrySettings>();
        m_geometry_converter = std::make_shared<GeometryConverter>(m_ifc_model, geom_settings);
        
        // Convert geometry
        m_geometry_converter->convertGeometry();
        
        // Convert model to JSON
        return convertModelToJson(m_ifc_model);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["error"] = "Failed to load IFC file: " + std::string(e.what());
        return error_response;
    }
}

json IfcParser::convertModelToJson(std::shared_ptr<BuildingModel> model) {
    json result;
    
    // Get basic model information
    std::shared_ptr<IfcProject> project = model->getIfcProject();
    if (project) {
        result["project"] = convertObjectToJson(project, std::set<int>());
    }
    
    // Get all entities
    const std::map<int, std::shared_ptr<BuildingObject>>& map_objects = model->getMapIfcObjects();
    json entities = json::array();
    
    for (const auto& pair : map_objects) {
        std::shared_ptr<IfcObjectDefinition> obj_def = 
            std::dynamic_pointer_cast<IfcObjectDefinition>(pair.second);
        if (obj_def) {
            std::set<int> visited;
            json entity_json = convertObjectToJson(obj_def, visited);
            if (!entity_json.is_null()) {
                entities.push_back(entity_json);
            }
        }
    }
    
    result["entities"] = entities;
    result["hierarchy"] = getBuildingHierarchy();
    
    return result;
}

json IfcParser::convertObjectToJson(std::shared_ptr<IfcObjectDefinition> obj, std::set<int>& visited) {
    if (!obj || visited.find(obj->m_tag) != visited.end()) {
        return json();
    }
    
    visited.insert(obj->m_tag);
    
    json obj_json;
    obj_json["id"] = obj->m_tag;
    obj_json["type"] = EntityFactory::getStringForClassID(obj->classID());
    
    // Basic attributes
    if (obj->m_GlobalId) {
        obj_json["globalId"] = obj->m_GlobalId->m_value;
    }
    
    if (obj->m_Name) {
        obj_json["name"] = obj->m_Name->m_value;
    }
    
    if (obj->m_Description) {
        obj_json["description"] = obj->m_Description->m_value;
    }
    
    // Add properties
    obj_json["properties"] = extractProperties(obj);
    
    // Add geometry if available
    if (m_geometry_converter) {
        const std::unordered_map<std::string, std::shared_ptr<ProductShapeData>>& shape_data = 
            m_geometry_converter->getShapeInputData();
        
        auto it = shape_data.find(std::to_string(obj->m_tag));
        if (it != shape_data.end()) {
            obj_json["geometry"] = extractGeometry(it->second);
        }
    }
    
    return obj_json;
}

json IfcParser::extractGeometry(std::shared_ptr<ProductShapeData> shapeData) {
    json geom_json;
    
    if (!shapeData) {
        return geom_json;
    }
    
    // Transform matrix
    carve::math::Matrix transform = shapeData->getTransform();
    json transform_json = json::array();
    for (int i = 0; i < 4; ++i) {
        json row = json::array();
        for (int j = 0; j < 4; ++j) {
            row.push_back(transform.m[i][j]);
        }
        transform_json.push_back(row);
    }
    geom_json["transform"] = transform_json;
    
    // Mesh data
    json meshes = json::array();
    for (auto geometricItem : shapeData->getGeometricItems()) {
        json mesh_json;
        json vertices = json::array();
        json faces = json::array();
        
        // Process closed meshes
        for (auto meshset : geometricItem->m_meshsets) {
            for (auto mesh : meshset->meshes) {
                for (auto face : mesh->faces) {
                    json face_indices = json::array();
                    carve::mesh::Edge<3>* edge = face->edge;
                    
                    for (size_t i = 0; i < face->n_edges; ++i) {
                        carve::mesh::Vertex<3>* vertex = edge->vert;
                        carve::geom::vector<3> point = vertex->v;
                        
                        // Add vertex
                        json vertex_json = json::array();
                        vertex_json.push_back(point.x);
                        vertex_json.push_back(point.y);
                        vertex_json.push_back(point.z);
                        vertices.push_back(vertex_json);
                        
                        face_indices.push_back(vertices.size() - 1);
                        edge = edge->next;
                    }
                    faces.push_back(face_indices);
                }
            }
        }
        
        mesh_json["vertices"] = vertices;
        mesh_json["faces"] = faces;
        meshes.push_back(mesh_json);
    }
    
    geom_json["meshes"] = meshes;
    return geom_json;
}

json IfcParser::extractProperties(std::shared_ptr<IfcObjectDefinition> obj) {
    json properties;
    
    // Basic object properties
    properties["entityId"] = obj->m_tag;
    properties["className"] = EntityFactory::getStringForClassID(obj->classID());
    
    // Add more specific properties based on object type
    // This can be extended for specific IFC types
    
    return properties;
}

json IfcParser::getEntityGeometry(const std::string& entityId) {
    if (!m_geometry_converter) {
        return json();
    }
    
    const std::unordered_map<std::string, std::shared_ptr<ProductShapeData>>& shape_data = 
        m_geometry_converter->getShapeInputData();
    
    auto it = shape_data.find(entityId);
    if (it != shape_data.end()) {
        return extractGeometry(it->second);
    }
    
    return json();
}

json IfcParser::getEntitiesByType(const std::string& ifcType) {
    json entities = json::array();
    
    if (!m_ifc_model) {
        return entities;
    }
    
    const std::map<int, std::shared_ptr<BuildingObject>>& map_objects = m_ifc_model->getMapIfcObjects();
    
    for (const auto& pair : map_objects) {
        std::shared_ptr<IfcObjectDefinition> obj_def = 
            std::dynamic_pointer_cast<IfcObjectDefinition>(pair.second);
        
        if (obj_def && EntityFactory::getStringForClassID(obj_def->classID()) == ifcType) {
            std::set<int> visited;
            json entity_json = convertObjectToJson(obj_def, visited);
            if (!entity_json.is_null()) {
                entities.push_back(entity_json);
            }
        }
    }
    
    return entities;
}

json IfcParser::getBuildingHierarchy() {
    if (!m_ifc_model) {
        return json();
    }
    
    std::shared_ptr<IfcProject> project = m_ifc_model->getIfcProject();
    if (!project) {
        return json();
    }
    
    std::set<int> visited;
    return convertObjectToJson(project, visited);
}