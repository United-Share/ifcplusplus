local ifc = std.extVar('ifc');

{
  // Building Summary Report
  title: "IFC Building Summary",
  generated: std.extVar('timestamp'),
  
  // Project information
  project: if std.objectHas(ifc, 'project') then {
    name: if std.objectHas(ifc.project, 'name') then ifc.project.name else 'Unknown Project',
    description: if std.objectHas(ifc.project, 'description') then ifc.project.description else '',
    globalId: if std.objectHas(ifc.project, 'globalId') then ifc.project.globalId else '',
  } else {},
  
  // Overall statistics
  statistics: {
    totalEntities: std.length(ifc.entities),
    entitiesWithGeometry: std.length([
      entity for entity in ifc.entities 
      if std.objectHas(entity, 'geometry')
    ]),
    entitiesWithNames: std.length([
      entity for entity in ifc.entities 
      if std.objectHas(entity, 'name') && entity.name != ''
    ]),
  },
  
  // Entity type breakdown
  entityTypes: {
    [entityType]: {
      count: std.length([entity for entity in ifc.entities if entity.type == entityType]),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == entityType && std.objectHas(entity, 'geometry')
      ]),
      withNames: std.length([
        entity for entity in ifc.entities 
        if entity.type == entityType && std.objectHas(entity, 'name') && entity.name != ''
      ]),
      entities: [
        {
          id: entity.id,
          globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
          name: if std.objectHas(entity, 'name') then entity.name else '',
          hasGeometry: std.objectHas(entity, 'geometry'),
        }
        for entity in ifc.entities
        if entity.type == entityType
      ]
    }
    for entityType in std.set([entity.type for entity in ifc.entities])
  },
  
  // Spatial structure (buildings, storeys, spaces)
  spatialStructure: {
    buildings: [
      {
        id: entity.id,
        name: if std.objectHas(entity, 'name') then entity.name else 'Unnamed Building',
        globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
        hasGeometry: std.objectHas(entity, 'geometry'),
      }
      for entity in ifc.entities
      if entity.type == 'IfcBuilding'
    ],
    
    storeys: [
      {
        id: entity.id,
        name: if std.objectHas(entity, 'name') then entity.name else 'Unnamed Storey',
        globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
        hasGeometry: std.objectHas(entity, 'geometry'),
      }
      for entity in ifc.entities
      if entity.type == 'IfcBuildingStorey'
    ],
    
    spaces: [
      {
        id: entity.id,
        name: if std.objectHas(entity, 'name') then entity.name else 'Unnamed Space',
        globalId: if std.objectHas(entity, 'globalId') then entity.globalId else '',
        hasGeometry: std.objectHas(entity, 'geometry'),
      }
      for entity in ifc.entities
      if entity.type == 'IfcSpace'
    ],
  },
  
  // Building elements summary
  buildingElements: {
    walls: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcWall']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcWall' && std.objectHas(entity, 'geometry')
      ]),
    },
    
    slabs: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcSlab']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcSlab' && std.objectHas(entity, 'geometry')
      ]),
    },
    
    beams: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcBeam']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcBeam' && std.objectHas(entity, 'geometry')
      ]),
    },
    
    columns: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcColumn']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcColumn' && std.objectHas(entity, 'geometry')
      ]),
    },
    
    doors: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcDoor']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcDoor' && std.objectHas(entity, 'geometry')
      ]),
    },
    
    windows: {
      count: std.length([entity for entity in ifc.entities if entity.type == 'IfcWindow']),
      withGeometry: std.length([
        entity for entity in ifc.entities 
        if entity.type == 'IfcWindow' && std.objectHas(entity, 'geometry')
      ]),
    },
  },
  
  // Geometry statistics
  geometryStats: {
    local entitiesWithGeom = [entity for entity in ifc.entities if std.objectHas(entity, 'geometry')],
    
    totalMeshes: std.foldl(
      function(acc, entity) acc + std.length(entity.geometry.meshes),
      entitiesWithGeom,
      0
    ),
    
    totalVertices: std.foldl(
      function(acc, entity) acc + std.foldl(
        function(acc2, mesh) acc2 + std.length(mesh.vertices),
        entity.geometry.meshes,
        0
      ),
      entitiesWithGeom,
      0
    ),
    
    totalFaces: std.foldl(
      function(acc, entity) acc + std.foldl(
        function(acc2, mesh) acc2 + std.length(mesh.faces),
        entity.geometry.meshes,
        0
      ),
      entitiesWithGeom,
      0
    ),
    
    averageVerticesPerEntity: {
      local totalVerts = self.totalVertices,
      local entityCount = std.length(entitiesWithGeom),
      value: if entityCount > 0 then totalVerts / entityCount else 0,
    },
  },
  
  // Quality metrics
  qualityMetrics: {
    geometryCompleteness: {
      percentage: if self.statistics.totalEntities > 0 then 
        (self.statistics.entitiesWithGeometry / self.statistics.totalEntities) * 100 
      else 0,
      status: if self.percentage >= 80 then 'Good'
              else if self.percentage >= 50 then 'Moderate'
              else 'Poor',
    },
    
    namingCompleteness: {
      percentage: if self.statistics.totalEntities > 0 then 
        (self.statistics.entitiesWithNames / self.statistics.totalEntities) * 100 
      else 0,
      status: if self.percentage >= 90 then 'Excellent'
              else if self.percentage >= 70 then 'Good'
              else if self.percentage >= 50 then 'Moderate'
              else 'Poor',
    },
  },
  
  // Metadata
  metadata: {
    templateVersion: "1.0",
    templateType: "building-summary",
    generatedBy: "IFC JSON Renderer",
    processingTimestamp: std.extVar('timestamp'),
  }
}