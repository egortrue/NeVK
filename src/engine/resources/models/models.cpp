#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "models.h"

Models::Models() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  tinyobj::LoadObj(&attrib, &shapes, &materials,
                   &warn, &err, "misc/models/teapot.obj", "misc/models/", true);

  for (auto& shape : shapes) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
      int fv = shape.mesh.num_face_vertices[f];
      for (size_t v = 0; v < fv; v++) {
        vertex_t vertex{};
        auto idx = shape.mesh.indices[index_offset + v];

        cube.indices.push_back(cube.vertices.size() / 5);

        cube.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
        cube.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
        cube.vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

        cube.vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
        cube.vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
      }
      index_offset += fv;
    }
  }

  for (int j = 0; j < 36; ++j) {
    for (int i = 0; i < 5; ++i)
      std::cout << cube.vertices[5 * j + i] << " ";
    std::cout << std::endl;
  }

  for (int j = 0; j < 12; ++j) {
    for (int i = 0; i < 3; ++i)
      std::cout << cube.indices[3 * j + i] << " ";
    std::cout << std::endl;
  }
}

Models::~Models() {
}
