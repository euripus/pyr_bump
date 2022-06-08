#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <string_view>
#include <vector>

struct Mesh
{
    std::vector<glm::vec3>      pos;
    std::vector<glm::vec3>      normals;
    std::vector<glm::vec3>      tangent;
    std::vector<glm::vec3>      bitangent;
    std::vector<glm::vec2>      tex;
    std::vector<unsigned short> indicies;
};

bool LoadMesh(std::string const & fname, Mesh & out);

#endif   // MESH_H
