# IFC JSON Renderer

A C++ REST API server that converts IFC (Industry Foundation Classes) files to JSON and renders them using Jsonnet templates on-demand.

## Features

- **IFC to JSON Conversion**: Parse IFC files and convert them to structured JSON format
- **Geometry Extraction**: Extract 3D geometry data including meshes, vertices, and faces
- **Building Hierarchy**: Navigate and extract building spatial structure
- **Entity Querying**: Query specific entity types (walls, slabs, beams, etc.)
- **Jsonnet Template Rendering**: Use Jsonnet templates to transform and analyze IFC data
- **REST API**: Full RESTful API with CORS support
- **On-Demand Processing**: Load and process IFC files dynamically
- **Caching**: Smart caching of loaded IFC data for performance

## Architecture

The application consists of four main components:

1. **IFC Parser** (`ifc_parser.cpp/.h`): Converts IFC files to JSON using IFC++
2. **Jsonnet Renderer** (`jsonnet_renderer.cpp/.h`): Processes Jsonnet templates with IFC data
3. **REST Endpoints** (`rest_endpoints.cpp/.h`): Provides HTTP API using Crow framework
4. **Main Application** (`main.cpp`): Ties everything together

## Dependencies

- **IFC++**: For IFC file parsing and geometry processing
- **Crow**: C++ web framework for REST API
- **Jsonnet**: Template language for data transformation
- **nlohmann/json**: JSON library for C++
- **CMake**: Build system

## Building

### Prerequisites

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install cmake build-essential git

# Or on macOS with Homebrew
brew install cmake git
```

### Build Steps

```bash
# Clone and build from the main project directory
mkdir build
cd build
cmake ..
make IfcJsonNetRenderer

# Or build specifically this example
cd examples/IfcJsonNetRenderer
mkdir build
cd build
cmake ..
make
```

## Usage

### Starting the Server

```bash
# Start server on default port 8080
./IfcJsonNetRenderer

# Start server on custom port
./IfcJsonNetRenderer -p 3000

# Show help
./IfcJsonNetRenderer --help

# Show version
./IfcJsonNetRenderer --version
```

### API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | API information |
| GET | `/api/status` | Server status and loaded files |
| POST | `/api/ifc/load` | Load an IFC file |
| GET | `/api/ifc/entities/{type}` | Get entities by type |
| GET | `/api/ifc/entity/{id}/geometry` | Get entity geometry |
| GET | `/api/ifc/hierarchy` | Get building hierarchy |
| POST | `/api/render/template` | Render Jsonnet template |
| POST | `/api/render/template/file` | Render template file |
| GET | `/api/render/templates/default` | Get default template |
| GET | `/api/render/templates/element/{type}` | Get element type template |

### Examples

#### Load an IFC File

```bash
curl -X POST http://localhost:8080/api/ifc/load \
  -H "Content-Type: application/json" \
  -d '{"filename": "example.ifc"}'
```

#### Get All Walls

```bash
curl http://localhost:8080/api/ifc/entities/IfcWall
```

#### Render a Template

```bash
curl -X POST http://localhost:8080/api/render/template \
  -H "Content-Type: application/json" \
  -d '{
    "template": "local ifc = std.extVar(\"ifc\"); { totalEntities: std.length(ifc.entities) }",
    "variables": {
      "author": "John Doe"
    }
  }'
```

#### Use Pre-built Templates

```bash
# Get wall analysis
curl -X POST http://localhost:8080/api/render/template/file \
  -H "Content-Type: application/json" \
  -d '{"templateFile": "templates/wall-analysis.jsonnet"}'

# Get building summary
curl -X POST http://localhost:8080/api/render/template/file \
  -H "Content-Type: application/json" \
  -d '{"templateFile": "templates/building-summary.jsonnet"}'
```

## Jsonnet Templates

The application includes several pre-built Jsonnet templates:

### Default Template

Basic IFC information including:
- Project details
- Entity counts and types
- Entities with geometry
- Summary statistics

### Wall Analysis Template (`templates/wall-analysis.jsonnet`)

Detailed wall analysis including:
- Wall count and geometry statistics
- Bounding box calculations
- Area estimations
- Largest wall identification

### Building Summary Template (`templates/building-summary.jsonnet`)

Comprehensive building overview including:
- Spatial structure (buildings, storeys, spaces)
- Building elements breakdown
- Geometry statistics
- Quality metrics

### Custom Templates

You can create your own Jsonnet templates. The IFC data is available via `std.extVar('ifc')`:

```jsonnet
local ifc = std.extVar('ifc');

{
  // Your custom analysis
  wallCount: std.length([
    entity for entity in ifc.entities 
    if entity.type == 'IfcWall'
  ]),
  
  // Access project info
  projectName: ifc.project.name,
  
  // Process geometry
  entitiesWithMeshes: [
    entity.id for entity in ifc.entities 
    if std.objectHas(entity, 'geometry')
  ]
}
```

## IFC Data Structure

The JSON representation of IFC data follows this structure:

```json
{
  "project": {
    "id": 1,
    "type": "IfcProject",
    "globalId": "...",
    "name": "Project Name",
    "description": "..."
  },
  "entities": [
    {
      "id": 123,
      "type": "IfcWall",
      "globalId": "1a2b3c4d-...",
      "name": "Wall-001",
      "description": "External wall",
      "properties": {
        "entityId": 123,
        "className": "IfcWall"
      },
      "geometry": {
        "transform": [[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],
        "meshes": [
          {
            "vertices": [[x,y,z], ...],
            "faces": [[v1,v2,v3], ...]
          }
        ]
      }
    }
  ],
  "hierarchy": {
    // Spatial hierarchy structure
  }
}
```

## Performance Considerations

- **Caching**: Loaded IFC files are cached in memory for subsequent requests
- **On-Demand Loading**: IFC files are only parsed when explicitly requested
- **Geometry Processing**: Geometry conversion can be resource-intensive for large files
- **Template Rendering**: Jsonnet templates are compiled and executed for each request

## Error Handling

The API provides detailed error responses:

```json
{
  "error": "Error message description",
  "timestamp": "2024-01-15T10:30:00Z"
}
```

Common error scenarios:
- File not found
- Invalid IFC file format
- Jsonnet template syntax errors
- Memory limitations for large files

## Extending the Application

### Adding New Endpoints

1. Add endpoint handler in `rest_endpoints.h/cpp`
2. Register route in `setupEndpoints()`
3. Implement business logic

### Custom IFC Processing

1. Extend `IfcParser` class with new methods
2. Add entity-specific extraction logic
3. Update JSON output structure

### New Template Functions

1. Add static methods to `JsonnetRenderer`
2. Provide built-in templates for common use cases
3. Document template variables and structure

## License

This project uses the same license as the main IFC++ project.

## Contributing

1. Follow the existing code structure and naming conventions
2. Add comprehensive error handling
3. Update documentation for new features
4. Test with various IFC file formats

## Support

For issues and questions:
1. Check the main IFC++ documentation
2. Review API endpoint responses for error details
3. Validate IFC files with other tools if parsing fails
4. Test Jsonnet templates separately if rendering fails