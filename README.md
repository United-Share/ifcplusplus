ABOUT IFC++
=============

This repository is more or less archived.<br>
If you need support or customization for existing projects, it is still available (please contact info at ifcquery.com)<br>
For new projects, I recommend Web-ifc, since it fulfills the same purpose, plus the ability to run in WASM, see below.<br>

## GitHub Actions - Automated Building

[![Build Examples](https://github.com/ifcquery/ifcplusplus/actions/workflows/build-examples.yml/badge.svg)](https://github.com/ifcquery/ifcplusplus/actions/workflows/build-examples.yml)

This repository includes a comprehensive GitHub Action workflow that automatically builds all example applications across multiple platforms. The workflow is designed to ensure cross-platform compatibility and provide ready-to-use binaries.

### Available Examples

The GitHub Action builds the following examples:

1. **Console Applications**:
   - `CreateIfcWallAndWriteFile` - Creates a simple IFC wall and writes to file
   - `LoadFileExample` - Loads and processes IFC files

2. **GUI Application**:
   - `SimpleViewerExampleQt` - Qt-based 3D viewer for IFC files (requires Qt6 + OpenSceneGraph)

3. **Web Service**:
   - `IfcJsonNetRenderer` - REST API service for IFC to JSON conversion with template rendering

### Build Matrix

| Platform | Console Examples | Qt Viewer | JSON Renderer |
|----------|------------------|-----------|---------------|
| Ubuntu Latest | ✅ (Debug/Release) | ✅ (Release) | ✅ (Release) |
| Windows Latest | ✅ (Debug/Release) | ✅ (Release) | ✅ (Release) |
| macOS Latest | ✅ (Debug/Release) | ❌ | ✅ (Release) |

### How to Use the GitHub Action

#### Automatic Triggers
The build workflow runs automatically on:
- Push to `main`, `master`, or `develop` branches
- Pull requests targeting these branches
- Manual workflow dispatch

#### Manual Execution
1. Go to the **Actions** tab in your GitHub repository
2. Select **"Build Examples"** workflow
3. Click **"Run workflow"** button
4. Choose the branch you want to build from
5. Click **"Run workflow"** to start the build

#### Accessing Built Artifacts
1. Navigate to the **Actions** tab
2. Click on a completed workflow run
3. Scroll down to the **Artifacts** section
4. Download the artifacts for your platform:
   - `console-examples-{platform}-{build_type}` - Console applications
   - `qt-viewer-{platform}-Release` - Qt viewer application
   - `json-renderer-{platform}-Release` - JSON renderer service

#### Local Development Setup
If you want to build locally, the GitHub Action serves as a reference for required dependencies:

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build libboost-all-dev pkg-config

# For Qt viewer (additional):
sudo apt-get install -y libopenscenegraph-dev libopenthreads-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev

# Install Qt6 from official installer or:
sudo apt-get install -y qt6-base-dev qt6-tools-dev qt6-widgets-dev
```

**macOS:**
```bash
brew install cmake ninja boost pkg-config

# For Qt viewer:
brew install qt@6 open-scene-graph
```

**Windows:**
- Install Visual Studio 2022 with C++ development tools
- Use vcpkg for dependencies:
```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install boost openscenegraph
```

#### Build Commands
```bash
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_CONSOLE_APPLICATION=ON -DBUILD_VIEWER_APPLICATION=ON

# Build specific examples
cmake --build . --target CreateIfcWallAndWriteFile
cmake --build . --target LoadFileExample
cmake --build . --target SimpleViewerExampleQt    # Requires Qt6 + OpenSceneGraph
cmake --build . --target IfcJsonNetRenderer       # Downloads dependencies automatically
```

### Dependencies Handled Automatically

The GitHub Action automatically installs and configures:
- **CMake** and build tools
- **Boost** libraries
- **Qt6** (QtBase, QtWidgets, QtOpenGL)
- **OpenSceneGraph** and OpenThreads
- **Crow framework** (via CMake FetchContent)
- **jsonnet** library (via CMake FetchContent)
- **nlohmann/json** (via CMake FetchContent)

### Troubleshooting

If builds fail, check the workflow logs in the Actions tab. Common issues:
- Missing submodules (ensure `submodules: recursive` in checkout)
- Qt6 path configuration on different platforms
- OpenSceneGraph version compatibility
- Network issues during dependency download

## Web-ifc as alternative to IFC++

As an alternative to IFC++, there is a great new project called web-ifc (https://github.com/ThatOpen/engine_web-ifc). It does not have an object oriented approach for IFC entities, instead it has a tape reader, so the STEP file content is kept as-is, just with tokens inserted before each attribute, which allows positioning the read cursor to read all entities and attributes.<br>
Web-ifc is so light weight, it can be directly compiled into a C++ console or GUI application, or linked as a library on Windows or Linux. It even compiles and runs efficiently as WebAssembly.<br><br>

If you want to benefit from my experience how to implement web-ifc in various applications, please contact info at ifcquery.com<br><br>

One example of a light weight application based on web-ifc is https://github.com/ifcquery/IfcSplitAndMerge

