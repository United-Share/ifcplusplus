# Test-Driven Development mit Catch2

Dieses Verzeichnis enthält die Testsuite für das IfcJsonNetRenderer-Projekt unter Verwendung des Catch2 Test-Frameworks.

## Überblick

Wir verwenden Test-Driven Development (TDD) mit folgender Struktur:

- **Unit Tests**: Testen einzelne Komponenten isoliert
- **Integration Tests**: Testen das Zusammenspiel der Komponenten
- **Performance Tests**: Messen und validieren Performance-Anforderungen

## Test-Struktur

```
tests/
├── CMakeLists.txt          # Build-Konfiguration für Tests
├── test_main.cpp           # Haupt-Testdatei (Catch2 main)
├── test_ifc_parser.cpp     # Tests für IFC Parser
├── test_jsonnet_renderer.cpp # Tests für Jsonnet Renderer
├── test_rest_endpoints.cpp # Tests für REST API Endpoints
└── test_integration.cpp    # Integrationstests
```

## TDD-Workflow

### 1. Red Phase - Test schreiben
```cpp
TEST_CASE("Neue Funktionalität", "[komponente]") {
    SECTION("Sollte X machen wenn Y") {
        // Arrange
        MyClass obj;
        
        // Act
        auto result = obj.newMethod(input);
        
        // Assert
        REQUIRE(result == expected);
    }
}
```

### 2. Green Phase - Implementierung
Implementiere nur genug Code, um den Test zu bestehen:
```cpp
ReturnType MyClass::newMethod(InputType input) {
    // Minimale Implementierung
    return expected;
}
```

### 3. Refactor Phase - Code verbessern
Verbessere den Code, während die Tests grün bleiben.

## Tests ausführen

### Alle Tests
```bash
cd build
cmake .. -DBUILD_TESTING=ON
make
ctest
```

### Spezifische Tests
```bash
# Nur Unit Tests
./IfcJsonNetRenderer_tests [ifc_parser]

# Nur Integrationstests
./IfcJsonNetRenderer_tests [integration]

# Mit detaillierter Ausgabe
./IfcJsonNetRenderer_tests -s
```

### Performance Tests
```bash
# Performance-Tests sind standardmäßig deaktiviert
./IfcJsonNetRenderer_tests [!benchmark]
```

### Code Coverage
```bash
# Mit Coverage kompilieren
cmake .. -DCODE_COVERAGE=ON
make
./IfcJsonNetRenderer_tests

# Coverage-Report generieren
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage-report
```

## Best Practices

### 1. Test-Fixtures verwenden
```cpp
class MyTestFixture {
protected:
    // Setup Code
    MyTestFixture() {
        // Initialisierung
    }
    
    // Teardown Code
    ~MyTestFixture() {
        // Aufräumen
    }
    
    // Gemeinsame Test-Daten
    TestData data;
};
```

### 2. Aussagekräftige Test-Namen
```cpp
TEST_CASE("Component: Was getestet wird", "[tags]") {
    SECTION("Spezifisches Verhalten unter bestimmten Bedingungen") {
        // Test
    }
}
```

### 3. AAA-Pattern (Arrange-Act-Assert)
```cpp
SECTION("Test Beispiel") {
    // Arrange - Setup
    auto input = createTestInput();
    
    // Act - Ausführung
    auto result = functionUnderTest(input);
    
    // Assert - Verifizierung
    REQUIRE(result.isValid());
    REQUIRE(result.value() == expected);
}
```

### 4. Test-Tags verwenden
- `[unit]` - Unit Tests
- `[integration]` - Integrationstests
- `[performance]` - Performance Tests
- `[slow]` - Langsame Tests
- `[!benchmark]` - Benchmark Tests (standardmäßig deaktiviert)

### 5. Matcher verwenden
```cpp
#include <catch2/matchers/catch_matchers_string.hpp>

REQUIRE_THAT(result, Contains("expected substring"));
REQUIRE_THAT(result, StartsWith("prefix"));
```

## Continuous Integration

Die Tests werden automatisch bei jedem Push ausgeführt. Siehe `.github/workflows/` für die CI-Konfiguration.

## Debugging von Tests

### Mit GDB
```bash
gdb ./IfcJsonNetRenderer_tests
(gdb) break test_ifc_parser.cpp:42
(gdb) run "[ifc_parser]"
```

### Mit Valgrind
```bash
valgrind --leak-check=full ./IfcJsonNetRenderer_tests
```

## Mocking

Für komplexere Tests können Sie Mocking-Frameworks wie FakeIt verwenden:

```cpp
#include <fakeit.hpp>

TEST_CASE("Mit Mock") {
    Mock<IMyInterface> mock;
    When(Method(mock, getValue)).Return(42);
    
    auto& instance = mock.get();
    REQUIRE(instance.getValue() == 42);
}
```

## Weitere Ressourcen

- [Catch2 Dokumentation](https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md)
- [TDD Best Practices](https://www.agilealliance.org/glossary/tdd/)
- [C++ Testing Patterns](https://www.modernescpp.com/index.php/c-core-guidelines-rules-for-testing)