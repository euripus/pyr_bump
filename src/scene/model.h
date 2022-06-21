#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AABB.h"

struct ModelComponent
{
    struct JointNode
    {
        uint32_t    parent;
        uint32_t    index;
        std::string name;

        std::vector<glm::quat> rot;     // absolute transform matrix for animation
        std::vector<glm::vec3> trans;   // vec.size() == numFrames
    };

    struct Mesh
    {
		struct Weight
		{
			uint32_t jointIndex;
			float    w;
		};
		using Weights = std::array<Weight, 4>;
	    
		//dynamic data
        std::vector<glm::vec3>              pos;
        std::vector<glm::vec3>              normal;
        std::vector<glm::vec3>              tangent;
        std::vector<glm::vec3>              bitangent;
		//static data
		std::vector<Weights>             weights;
        std::vector<sglm::vec2>     tex_coords;      
        std::vector<uint32_t> indexes;
		
		AABB                                bbox;
    };

	std::vector<Mesh>  meshes;
    std::vector<JointNode> joints;
    std::vector<AABB>      bboxes;
    uint32_t               numFrames;
    float                  frameRate;
};


#endif
