#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <ifcpp/model/BuildingModel.h>
#include <ifcpp/reader/ReaderSTEP.h>
#include <ifcpp/geometry/GeometryConverter.h>
#include <ifcpp/IFC4X3/include/IfcProject.h>
#include <ifcpp/IFC4X3/include/IfcObjectDefinition.h>

/**
 * IFC Parser class that converts IFC files to JSON representation
 */
class IfcParser {
public:
    IfcParser();
    ~IfcParser();

    /**
     * Load IFC file and test basic functionality
     * @param filename Path to the IFC file
     * @return true if successful
     */
    bool loadIfcFile(const std::string& filename);

    /**
     * Convert IFC model to string representation
     * @param model The IFC building model
     * @return string representation
     */
    std::string convertModelToString(std::shared_ptr<BuildingModel> model);

    /**
     * Get basic entity information
     * @param entityId The entity ID
     * @return basic info string
     */
    std::string getEntityInfo(const std::string& entityId);

    /**
     * Get all entities of a specific type
     * @param ifcType The IFC type (e.g., "IfcWall", "IfcSlab")
     * @return count of entities
     */
    int getEntitiesByTypeCount(const std::string& ifcType);

    /**
     * Get building hierarchy as string
     * @return hierarchy info
     */
    std::string getBuildingHierarchy();

private:
    std::shared_ptr<BuildingModel> m_ifc_model;
    std::shared_ptr<ReaderSTEP> m_step_reader;
    std::shared_ptr<GeometryConverter> m_geometry_converter;
    
    // Cache for converted entities
    std::map<std::string, std::string> m_entity_cache;
    
    /**
     * Convert an IFC object to string recursively
     * @param obj The IFC object
     * @param visited Set of visited objects to avoid cycles
     * @return string representation
     */
    std::string convertObjectToString(std::shared_ptr<IFC4X3::IfcObjectDefinition> obj, 
                            std::set<int>& visited);

    /**
     * Extract basic information for an entity
     * @param shapeData The shape data
     * @return string with basic information
     */
    std::string extractBasicInfo(std::shared_ptr<ProductShapeData> shapeData);

    /**
     * Convert properties to string
     * @param obj The IFC object
     * @return string with properties
     */
    std::string extractProperties(std::shared_ptr<IFC4X3::IfcObjectDefinition> obj);
};