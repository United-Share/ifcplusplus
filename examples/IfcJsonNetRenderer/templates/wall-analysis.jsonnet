local ifc = std.extVar('ifc');

{
  // Wall Analysis Report
  title: "IFC Wall Analysis Report",
  generated: std.extVar('timestamp'),
  
  // Summary statistics
  summary: {
    totalWalls: std.length([entity for entity in ifc.entities if entity.type == 'IfcWall']),
    wallsWithGeometry: std.length([
      entity for entity in ifc.entities 
      if entity.type == 'IfcWall' && std.objectHas(entity, 'geometry')
    ]),
    wallsWithNames: std.length([
      entity for entity in ifc.entities 
      if entity.type == 'IfcWall' && std.objectHas(entity, 'name') && entity.name != ''
    ]),
  },
  
  // Detailed wall information
  walls: [
    {
      id: wall.id,
      globalId: if std.objectHas(wall, 'globalId') then wall.globalId else '',
      name: if std.objectHas(wall, 'name') then wall.name else 'Unnamed Wall',
      description: if std.objectHas(wall, 'description') then wall.description else '',
      
      // Geometry analysis
      geometry: if std.objectHas(wall, 'geometry') then {
        hasMesh: true,
        meshCount: std.length(wall.geometry.meshes),
        totalVertices: std.foldl(
          function(acc, mesh) acc + std.length(mesh.vertices),
          wall.geometry.meshes,
          0
        ),
        totalFaces: std.foldl(
          function(acc, mesh) acc + std.length(mesh.faces),
          wall.geometry.meshes,
          0
        ),
        
        // Calculate approximate wall area (simplified)
        estimatedArea: if std.length(wall.geometry.meshes) > 0 then
          std.foldl(
            function(acc, mesh) acc + std.length(mesh.faces) * 0.5, // rough estimation
            wall.geometry.meshes,
            0
          )
        else 0,
        
        // Bounding box calculation
        boundingBox: if std.length(wall.geometry.meshes) > 0 then {
          local allVertices = std.flattenArrays([
            mesh.vertices for mesh in wall.geometry.meshes
          ]),
          min: if std.length(allVertices) > 0 then [
            std.foldl(function(acc, v) std.min(acc, v[0]), allVertices, 1e10),
            std.foldl(function(acc, v) std.min(acc, v[1]), allVertices, 1e10),
            std.foldl(function(acc, v) std.min(acc, v[2]), allVertices, 1e10),
          ] else [0, 0, 0],
          max: if std.length(allVertices) > 0 then [
            std.foldl(function(acc, v) std.max(acc, v[0]), allVertices, -1e10),
            std.foldl(function(acc, v) std.max(acc, v[1]), allVertices, -1e10),
            std.foldl(function(acc, v) std.max(acc, v[2]), allVertices, -1e10),
          ] else [0, 0, 0],
        } else { min: [0, 0, 0], max: [0, 0, 0] },
      } else {
        hasMesh: false,
        meshCount: 0,
        totalVertices: 0,
        totalFaces: 0,
        estimatedArea: 0,
        boundingBox: { min: [0, 0, 0], max: [0, 0, 0] },
      },
      
      // Properties
      properties: wall.properties,
    }
    for wall in ifc.entities
    if wall.type == 'IfcWall'
  ],
  
  // Analysis results
  analysis: {
    largestWall: {
      local wallsWithArea = [
        { id: w.id, name: w.name, area: w.geometry.estimatedArea }
        for w in self.walls
        if w.geometry.hasMesh
      ],
      
      id: if std.length(wallsWithArea) > 0 then
        std.foldl(
          function(acc, w) if w.area > acc.area then w else acc,
          wallsWithArea,
          wallsWithArea[0]
        ).id
      else null,
    },
    
    averageArea: {
      local wallsWithArea = [w.geometry.estimatedArea for w in self.walls if w.geometry.hasMesh],
      value: if std.length(wallsWithArea) > 0 then
        std.foldl(function(acc, area) acc + area, wallsWithArea, 0) / std.length(wallsWithArea)
      else 0,
    },
    
    totalEstimatedArea: std.foldl(
      function(acc, wall) acc + wall.geometry.estimatedArea,
      self.walls,
      0
    ),
  },
  
  // Metadata
  metadata: {
    templateVersion: "1.0",
    templateType: "wall-analysis",
    ifcEntitiesProcessed: std.length(ifc.entities),
    processingTimestamp: std.extVar('timestamp'),
  }
}