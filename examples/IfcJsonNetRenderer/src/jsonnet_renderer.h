#pragma once

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <libjsonnet.h>

using json = nlohmann::json;

/**
 * Jsonnet template renderer for IFC data
 */
class JsonnetRenderer {
public:
    JsonnetRenderer();
    ~JsonnetRenderer();

    /**
     * Render a Jsonnet template with IFC data
     * @param template_content The Jsonnet template content
     * @param ifc_data JSON data from IFC parser
     * @param external_vars Additional variables to pass to template
     * @return Rendered JSON output
     */
    json renderTemplate(const std::string& template_content, 
                       const json& ifc_data,
                       const std::map<std::string, std::string>& external_vars = {});

    /**
     * Load and render a Jsonnet template file
     * @param template_file Path to the Jsonnet template file
     * @param ifc_data JSON data from IFC parser
     * @param external_vars Additional variables to pass to template
     * @return Rendered JSON output
     */
    json renderTemplateFile(const std::string& template_file,
                           const json& ifc_data,
                           const std::map<std::string, std::string>& external_vars = {});

    /**
     * Set import callback for Jsonnet includes
     * @param import_callback Callback function for handling imports
     */
    void setImportCallback(JsonnetImportCallback* import_callback);

    /**
     * Add external variable
     * @param key Variable name
     * @param value Variable value
     */
    void addExternalVar(const std::string& key, const std::string& value);

    /**
     * Clear all external variables
     */
    void clearExternalVars();

    /**
     * Create a default template for IFC visualization
     * @return Default Jsonnet template string
     */
    static std::string getDefaultIfcTemplate();

    /**
     * Create a template for specific IFC element types
     * @param element_type The IFC element type (e.g., "IfcWall")
     * @return Jsonnet template for the element type
     */
    static std::string getElementTypeTemplate(const std::string& element_type);

private:
    JsonnetVm* m_vm;
    std::map<std::string, std::string> m_external_vars;

    /**
     * Setup the Jsonnet VM with IFC data
     * @param ifc_data JSON data to make available in templates
     */
    void setupVmWithIfcData(const json& ifc_data);

    /**
     * Handle Jsonnet errors
     * @param error_msg Error message from Jsonnet
     * @return JSON error response
     */
    json handleJsonnetError(const std::string& error_msg);
};