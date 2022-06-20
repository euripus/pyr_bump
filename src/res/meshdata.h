#ifndef MESHDATA_H
#define MESHDATA_H

#include <glm/glm.hpp>
#include <vector>

struct MeshData
{
    std::vector<glm::vec3>      pos;
    std::vector<glm::vec3>      normals;
    std::vector<glm::vec3>      tangent;
    std::vector<glm::vec3>      bitangent;
    std::vector<glm::vec2>      tex;
    std::vector<unsigned short> indicies;
};

bool LoadMesh(std::string const & fname, MeshData & out);

#endif   // MESHDATA_H
