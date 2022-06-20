#include "meshdata.h"
#include <fstream>
#include <sstream>
#include <algorithm>

bool LoadMesh(std::string const & fname, MeshData & out)
{
    std::ifstream ifile(fname, std::ios::in);

    if(!ifile)
        return false;

    std::string line;

    while(std::getline(ifile, line))
    {
        if(line.substr(0, 3) == "vtx")
        {
            std::istringstream ss(line.substr(3));
            glm::vec3          v;

            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            out.pos.push_back(std::move(v));
        }
        else if(line.substr(0, 3) == "vnr")
        {
            std::istringstream ss(line.substr(3));
            glm::vec3          v;

            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            out.normals.push_back(std::move(v));
        }
        else if(line.substr(0, 3) == "vtg")
        {
            std::istringstream ss(line.substr(3));
            glm::vec3          v;

            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            out.tangent.push_back(std::move(v));
        }
        else if(line.substr(0, 3) == "vbt")
        {
            std::istringstream ss(line.substr(3));
            glm::vec3          v;

            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            out.bitangent.push_back(std::move(v));
        }
        else if(line.substr(0, 3) == "tx0")
        {
            std::istringstream ss(line.substr(3));
            glm::vec2          v;

            ss >> v.x;
            ss >> v.y;
            out.tex.push_back(std::move(v));
        }
        else if(line.substr(0, 3) == "fcx")
        {
            std::istringstream ss(line.substr(3));
            glm::ivec3         v;

            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            out.indicies.push_back(static_cast<unsigned short>(v.x));
            out.indicies.push_back(static_cast<unsigned short>(v.y));
            out.indicies.push_back(static_cast<unsigned short>(v.z));
        }
    }

    ifile.close();

    auto const size = out.pos.size();
    if(size != out.normals.size() || size != out.tangent.size() || size != out.bitangent.size()
       || size != out.tex.size())
        return false;

    return true;
}
