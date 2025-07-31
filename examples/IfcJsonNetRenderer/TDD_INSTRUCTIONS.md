# Test-Driven Development mit Catch2 für IfcJsonNetRenderer

## Übersicht

Dieses Dokument beschreibt, wie Test-Driven Development (TDD) mit der Catch2-Bibliothek für das IfcJsonNetRenderer-Projekt implementiert wird.

## Installation und Setup

### 1. Catch2 Integration

Catch2 wird über CMake's FetchContent automatisch heruntergeladen:

```cmake
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)
```

### 2. Projekt-Struktur

```
examples/IfcJsonNetRenderer/
├── src/                    # Quellcode
│   ├── ifc_parser.cpp/h
│   ├── jsonnet_renderer.cpp/h
│   ├── rest_endpoints.cpp/h
│   └── main.cpp
├── tests/                  # Tests
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── test_ifc_parser.cpp
│   ├── test_jsonnet_renderer.cpp
│   └── test_rest_endpoints.cpp
└── CMakeLists.txt
```

## TDD-Workflow

### 1. Red Phase - Test schreiben

Zuerst schreiben Sie einen Test, der fehlschlägt:

```cpp
TEST_CASE("IFC Parser kann Wände extrahieren", "[ifc_parser]") {
    IfcParser parser;
    parser.parseFile("test.ifc");
    
    json walls = parser.getEntitiesByType("IfcWall");
    
    REQUIRE(walls["count"] > 0);
    REQUIRE(walls["entities"][0]["type"] == "IfcWall");
}
```

### 2. Green Phase - Minimale Implementierung

Implementieren Sie nur genug Code, um den Test zu bestehen:

```cpp
json IfcParser::getEntitiesByType(const std::string& type) {
    json result;
    result["type"] = type;
    result["entities"] = json::array();
    result["count"] = 0;
    
    // Minimal implementation
    for (auto entity : m_model->getMapIfcEntities()) {
        if (entity.second->classname() == type) {
            result["entities"].push_back(entityToJson(entity.second));
            result["count"] = result["entities"].size();
        }
    }
    
    return result;
}
```

### 3. Refactor Phase - Code verbessern

Optimieren Sie den Code, während alle Tests grün bleiben:

```cpp
json IfcParser::getEntitiesByType(const std::string& type) {
    json result = {
        {"type", type},
        {"entities", json::array()},
        {"count", 0}
    };
    
    auto& entities = result["entities"];
    
    std::for_each(
        m_model->getMapIfcEntities().begin(),
        m_model->getMapIfcEntities().end(),
        [&](const auto& pair) {
            if (pair.second->classname() == type) {
                entities.push_back(entityToJson(pair.second));
            }
        }
    );
    
    result["count"] = entities.size();
    return result;
}
```

## Test-Kategorien

### Unit Tests

Testen einzelne Funktionen isoliert:

```cpp
TEST_CASE("Jsonnet VM wird korrekt initialisiert", "[jsonnet][unit]") {
    JsonnetRenderer renderer;
    REQUIRE(renderer.isInitialized());
}
```

### Integration Tests

Testen das Zusammenspiel mehrerer Komponenten:

```cpp
TEST_CASE("REST API lädt IFC und rendert Template", "[integration]") {
    RestEndpoints endpoints;
    
    // Load IFC
    crow::request load_req;
    load_req.body = R"({"file_path": "test.ifc"})";
    auto load_resp = endpoints.handleLoadIfc(load_req);
    REQUIRE(load_resp.code == 200);
    
    // Render template
    crow::request render_req;
    render_req.body = R"({
        "template": "{ walls: std.length(std.extVar('ifc_data').entities) }"
    })";
    auto render_resp = endpoints.handleRenderTemplate(render_req);
    REQUIRE(render_resp.code == 200);
}
```

### Performance Tests

Messen die Ausführungszeit kritischer Operationen:

```cpp
TEST_CASE("IFC Parser Performance", "[performance][!benchmark]") {
    IfcParser parser;
    
    BENCHMARK("Parse 10MB IFC file") {
        return parser.parseFile("large_model.ifc");
    };
}
```

## Mock-Objekte und Test-Doubles

### Mock IFC Entities

```cpp
class MockIfcEntity : public IfcPPEntity {
public:
    MockIfcEntity(int id, const std::string& type) 
        : m_id(id), m_type(type) {}
    
    virtual const char* classname() const override { 
        return m_type.c_str(); 
    }
    
    virtual void getStepLine(std::stringstream& stream) const override {
        stream << "#" << m_id << "=" << m_type << "();";
    }
    
private:
    int m_id;
    std::string m_type;
};
```

### Test Fixtures

```cpp
class IfcParserFixture {
protected:
    IfcParser parser;
    shared_ptr<IfcPPModel> model;
    
    void SetUp() {
        model = make_shared<IfcPPModel>();
        parser.setModel(model);
    }
    
    void addMockEntity(int id, const std::string& type) {
        auto entity = make_shared<MockIfcEntity>(id, type);
        model->insertEntity(entity);
    }
};

TEST_CASE_METHOD(IfcParserFixture, "Parser mit Mock-Daten", "[ifc_parser]") {
    addMockEntity(1, "IfcWall");
    addMockEntity(2, "IfcDoor");
    
    auto walls = parser.getEntitiesByType("IfcWall");
    REQUIRE(walls["count"] == 1);
}
```

## Best Practices

### 1. Aussagekräftige Test-Namen

```cpp
// Gut
TEST_CASE("IFC Parser extrahiert Geometrie-Daten von Wänden korrekt", "[ifc_parser][geometry]")

// Schlecht
TEST_CASE("Test 1", "[test]")
```

### 2. Arrange-Act-Assert Pattern

```cpp
TEST_CASE("Jsonnet Renderer verarbeitet externe Variablen") {
    // Arrange
    JsonnetRenderer renderer;
    renderer.addExternalVar("project_name", "Test Project");
    std::string template_str = "{ name: std.extVar('project_name') }";
    
    // Act
    json result = renderer.renderTemplate(template_str, {});
    
    // Assert
    REQUIRE(result["name"] == "Test Project");
}
```

### 3. Test-Isolation

Jeder Test sollte unabhängig sein:

```cpp
TEST_CASE("REST Endpoints sind thread-safe", "[rest][concurrent]") {
    RestEndpoints endpoints;
    
    // Jeder Test bekommt seine eigene Instanz
    // Keine globalen Variablen oder geteilten Zustände
}
```

### 4. Datengetriebene Tests

```cpp
TEST_CASE("IFC Entity Types werden korrekt erkannt", "[ifc_parser]") {
    auto [entity_type, expected_category] = GENERATE(
        std::make_tuple("IfcWall", "BuildingElement"),
        std::make_tuple("IfcDoor", "BuildingElement"), 
        std::make_tuple("IfcSpace", "SpatialElement"),
        std::make_tuple("IfcProject", "Context")
    );
    
    SECTION(entity_type) {
        auto category = IfcParser::getEntityCategory(entity_type);
        REQUIRE(category == expected_category);
    }
}
```

## Continuous Integration

### GitHub Actions Workflow

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential
    
    - name: Build tests
      run: |
        cd examples/IfcJsonNetRenderer
        mkdir build && cd build
        cmake .. -DBUILD_TESTS=ON
        make -j4
    
    - name: Run tests
      run: |
        cd examples/IfcJsonNetRenderer/build
        ctest --output-on-failure
```

## Test-Befehle

### Alle Tests ausführen
```bash
cd build
ctest
```

### Nur bestimmte Test-Tags
```bash
./IfcJsonNetRendererTests "[ifc_parser]"
```

### Mit detaillierter Ausgabe
```bash
./IfcJsonNetRendererTests --reporter console --success
```

### Benchmarks ausführen
```bash
./IfcJsonNetRendererTests "[!benchmark]" --benchmark-samples 100
```

## Metriken und Coverage

### Code Coverage mit gcov/lcov

```bash
# Compile mit Coverage-Flags
cmake .. -DCMAKE_CXX_FLAGS="--coverage"
make

# Tests ausführen
./IfcJsonNetRendererTests

# Coverage-Report generieren
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info
```

### Ziel-Metriken

- **Code Coverage**: Mindestens 80% der Geschäftslogik
- **Test-Ausführungszeit**: Alle Unit-Tests < 1 Sekunde
- **Test-Verhältnis**: 2:1 (Tests zu Produktivcode)

## Beispiel: Vollständiger TDD-Zyklus

### Anforderung
"Der REST-Endpoint soll IFC-Dateien validieren können"

### 1. Test schreiben
```cpp
TEST_CASE("REST API validiert IFC-Dateien", "[rest][validation]") {
    RestEndpoints endpoints;
    crow::request req;
    
    SECTION("Gültige IFC-Datei") {
        req.body = R"({"file_path": "valid.ifc"})";
        auto response = endpoints.handleValidateIfc(req);
        
        auto result = json::parse(response.body);
        REQUIRE(response.code == 200);
        REQUIRE(result["valid"] == true);
        REQUIRE(result["errors"].empty());
    }
    
    SECTION("Ungültige IFC-Datei") {
        req.body = R"({"file_path": "invalid.ifc"})";
        auto response = endpoints.handleValidateIfc(req);
        
        auto result = json::parse(response.body);
        REQUIRE(response.code == 200);
        REQUIRE(result["valid"] == false);
        REQUIRE(!result["errors"].empty());
    }
}
```

### 2. Implementierung
```cpp
crow::response RestEndpoints::handleValidateIfc(const crow::request& req) {
    try {
        json request = json::parse(req.body);
        
        if (!request.contains("file_path")) {
            return createErrorResponse("Missing file_path");
        }
        
        std::string file_path = request["file_path"];
        auto validation_result = m_ifc_parser.validateFile(file_path);
        
        return createJsonResponse(validation_result);
        
    } catch (const std::exception& e) {
        return createErrorResponse(e.what());
    }
}
```

### 3. Refactoring
```cpp
crow::response RestEndpoints::handleValidateIfc(const crow::request& req) {
    return handleJsonRequest(req, [this](const json& request) {
        validateRequiredField(request, "file_path");
        
        const auto& file_path = request["file_path"].get<std::string>();
        return m_ifc_parser.validateFile(file_path);
    });
}
```

## Zusammenfassung

Test-Driven Development mit Catch2 ermöglicht:
- Höhere Code-Qualität durch frühzeitige Fehlererkennung
- Bessere Dokumentation durch aussagekräftige Tests
- Sicherheit bei Refactoring durch umfassende Test-Suite
- Schnellere Entwicklung durch klare Anforderungen

Die Kombination aus Crow (REST API), Jsonnet (Template Engine) und IFC++ (IFC Processing) mit einer soliden Test-Suite schafft eine robuste und erweiterbare Architektur für IFC-Datenverarbeitung.