#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Conditional compilation for jsonnet support
#ifdef ENABLE_JSONNET
#include <libjsonnet.h>
#endif

class JsonnetRenderer {
public:
    JsonnetRenderer();
    ~JsonnetRenderer();
    
    // Render template with IFC data
    json renderTemplate(const std::string& template_content,
                       const json& ifc_data,
                       const std::unordered_map<std::string, std::string>& external_vars = {});
    
    // Render template from file
    json renderTemplateFile(const std::string& template_file,
                           const json& ifc_data,
                           const std::unordered_map<std::string, std::string>& external_vars = {});
    
    // Add external variable
    void addExternalVar(const std::string& key, const std::string& value);
    
    // Clear external variables
    void clearExternalVars();
    
    // Static template getters
    static std::string getDefaultIfcTemplate();
    static std::string getElementTypeTemplate(const std::string& element_type);

private:
#ifdef ENABLE_JSONNET
    JsonnetVm* m_vm;
#endif
    std::unordered_map<std::string, std::string> m_external_vars;
    
    void setupVmWithIfcData(const json& ifc_data);
    void setImportCallback(JsonnetImportCallback* import_callback);
    json handleJsonnetError(const std::string& error_msg);
};