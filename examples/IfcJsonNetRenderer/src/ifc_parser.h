#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

#include <ifcpp/model/BuildingModel.h>
#include <ifcpp/reader/ReaderSTEP.h>
#include <ifcpp/geometry/GeometryConverter.h>
#include <ifcpp/IFC4X3/include/IfcProject.h>
#include <ifcpp/IFC4X3/include/IfcObjectDefinition.h>

using json = nlohmann::json;

/**
 * IFC Parser class that converts IFC files to JSON representation
 */
class IfcParser {
public:
    IfcParser();
    ~IfcParser();

    /**
     * Load IFC file and convert to JSON
     * @param filename Path to the IFC file
     * @return JSON representation of the IFC model
     */
    json loadIfcFile(const std::string& filename);

    /**
     * Convert IFC model to JSON
     * @param model The IFC building model
     * @return JSON representation
     */
    json convertModelToJson(std::shared_ptr<BuildingModel> model);

    /**
     * Get geometry data for a specific entity
     * @param entityId The entity ID
     * @return JSON with geometry data
     */
    json getEntityGeometry(const std::string& entityId);

    /**
     * Get all entities of a specific type
     * @param ifcType The IFC type (e.g., "IfcWall", "IfcSlab")
     * @return JSON array of entities
     */
    json getEntitiesByType(const std::string& ifcType);

    /**
     * Get building hierarchy as JSON
     * @return JSON tree structure
     */
    json getBuildingHierarchy();

private:
    std::shared_ptr<BuildingModel> m_ifc_model;
    std::shared_ptr<ReaderSTEP> m_step_reader;
    std::shared_ptr<GeometryConverter> m_geometry_converter;
    
    // Cache for converted entities
    std::map<std::string, json> m_entity_cache;
    
    /**
     * Convert an IFC object to JSON recursively
     * @param obj The IFC object
     * @param visited Set of visited objects to avoid cycles
     * @return JSON representation
     */
    json convertObjectToJson(std::shared_ptr<IFC4X3::IfcObjectDefinition> obj, 
                            std::set<int>& visited);

    /**
     * Extract geometry information for an entity
     * @param shapeData The shape data
     * @return JSON with geometry information
     */
    json extractGeometry(std::shared_ptr<ProductShapeData> shapeData);

    /**
     * Convert properties to JSON
     * @param obj The IFC object
     * @return JSON with properties
     */
    json extractProperties(std::shared_ptr<IFC4X3::IfcObjectDefinition> obj);
};