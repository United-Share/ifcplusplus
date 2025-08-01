#include "ifc_parser.h"
#include <iostream>
#include <sstream>
#include <ifcpp/IFC4X3/EntityFactory.h>
#include <ifcpp/geometry/GeometrySettings.h>

using namespace IFC4X3;

IfcParser::IfcParser() {
    m_ifc_model = std::make_shared<BuildingModel>();
    m_step_reader = std::make_shared<ReaderSTEP>();
    auto geom_settings = std::make_shared<GeometrySettings>();
    m_geometry_converter = std::make_shared<GeometryConverter>(m_ifc_model, geom_settings);
}

IfcParser::~IfcParser() {
    // Cleanup handled by unique_ptr
}

bool IfcParser::loadIfcFile(const std::string& filename) {
    try {
        m_step_reader->loadModelFromFile(filename, m_ifc_model);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading IFC file: " << e.what() << std::endl;
        return false;
    }
}

std::string IfcParser::convertModelToString(std::shared_ptr<BuildingModel> model) {
    if (!model) {
        return "Error: No model provided";
    }
    
    std::stringstream result;
    result << "IFC Model Information:\n";
    
    // Get project information
    if (std::shared_ptr<IfcProject> project = model->getIfcProject()) {
        std::set<int> visited;
        result << "Project: " << convertObjectToString(project, visited) << "\n";
    }
    
    // Get all entities
    const auto& map_entities = model->getMapIfcEntities();
    result << "Total entities: " << map_entities.size() << "\n";
    
    // Count by type
    std::map<std::string, int> type_counts;
    for (const auto& pair : map_entities) {
        if (pair.second) {
            std::string type_name = EntityFactory::getStringForClassID(pair.second->classID());
            type_counts[type_name]++;
        }
    }
    
    result << "Entity types:\n";
    for (const auto& type_pair : type_counts) {
        result << "  " << type_pair.first << ": " << type_pair.second << "\n";
    }
    
    return result.str();
}

std::string IfcParser::getEntityInfo(const std::string& entityId) {
    if (!m_ifc_model) {
        return "Error: No model loaded";
    }
    
    try {
        int id = std::stoi(entityId);
        const auto& entities = m_ifc_model->getMapIfcEntities();
        
        auto it = entities.find(id);
        if (it != entities.end() && it->second) {
            std::stringstream info;
            info << "Entity ID: " << id << "\n";
            info << "Type: " << EntityFactory::getStringForClassID(it->second->classID()) << "\n";
            return info.str();
        }
    } catch (const std::exception& e) {
        return "Error parsing entity ID: " + std::string(e.what());
    }
    
    return "Entity not found";
}

int IfcParser::getEntitiesByTypeCount(const std::string& ifcType) {
    if (!m_ifc_model) {
        return 0;
    }
    
    const auto& entities = m_ifc_model->getMapIfcEntities();
    int count = 0;
    
    for (const auto& pair : entities) {
        if (pair.second && EntityFactory::getStringForClassID(pair.second->classID()) == ifcType) {
            count++;
        }
    }
    
    return count;
}

std::string IfcParser::getBuildingHierarchy() {
    if (!m_ifc_model) {
        return "Error: No model loaded";
    }
    
    std::stringstream hierarchy;
    hierarchy << "Building Hierarchy:\n";
    
    auto project = m_ifc_model->getIfcProject();
    if (!project) {
        hierarchy << "No project found\n";
        return hierarchy.str();
    }
    
    std::set<int> visited;
    hierarchy << convertObjectToString(project, visited);
    
    return hierarchy.str();
}

std::string IfcParser::convertObjectToString(std::shared_ptr<IfcObjectDefinition> obj, std::set<int>& visited) {
    if (!obj) {
        return "null";
    }
    
    int obj_id = obj->m_tag;
    if (visited.find(obj_id) != visited.end()) {
        return "circular_reference_" + std::to_string(obj_id);
    }
    
    visited.insert(obj_id);
    
    std::stringstream result;
    result << EntityFactory::getStringForClassID(obj->classID()) << " (ID: " << obj_id << ")";
    
    return result.str();
}

std::string IfcParser::extractBasicInfo(std::shared_ptr<ProductShapeData> shapeData) {
    if (!shapeData) {
        return "No shape data";
    }
    
    return "Shape data available";
}

std::string IfcParser::extractProperties(std::shared_ptr<IfcObjectDefinition> obj) {
    if (!obj) {
        return "No object";
    }
    
    return std::string("Properties: ") + EntityFactory::getStringForClassID(obj->classID());
}