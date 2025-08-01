#include <iostream>
#include <fstream>
#include <memory>
#include <string>

// Only include basic headers for testing
#include <ifcpp/model/BuildingModel.h>
#include <ifcpp/reader/ReaderSTEP.h>
#include <ifcpp/IFC4X3/include/IfcProject.h>

using namespace IFC4X3;

int main(int argc, char* argv[]) {
    std::cout << "IFC++ Simple Test Application" << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        // Create building model and reader
        std::shared_ptr<BuildingModel> model = std::make_shared<BuildingModel>();
        std::shared_ptr<ReaderSTEP> reader = std::make_shared<ReaderSTEP>();
        
        std::cout << "BuildingModel and ReaderSTEP created successfully." << std::endl;
        
        // Test basic functionality
        if (argc > 1) {
            std::string filename = argv[1];
            std::cout << "Attempting to load IFC file: " << filename << std::endl;
            
            // Check if file exists
            std::ifstream file(filename);
            if (!file.good()) {
                std::cerr << "Error: Cannot open file " << filename << std::endl;
                return 1;
            }
            file.close();
            
            try {
                reader->loadModelFromFile(filename, model);
                std::cout << "File loaded successfully!" << std::endl;
                
                // Get basic information
                auto project = model->getIfcProject();
                if (project) {
                    std::cout << "Project found in model." << std::endl;
                } else {
                    std::cout << "No project found in model." << std::endl;
                }
                
                // Get entities count
                const auto& entities = model->getMapIfcEntities();
                std::cout << "Total entities in model: " << entities.size() << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "Error loading IFC file: " << e.what() << std::endl;
                return 1;
            }
        } else {
            std::cout << "Usage: " << argv[0] << " <ifc_file>" << std::endl;
            std::cout << "No file specified, testing basic functionality only." << std::endl;
        }
        
        std::cout << "Application completed successfully." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}