#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <array>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AABB.h"
#include "sceneentitybuilder.h"

struct ModelComponent
{
    struct JointNode
    {
        uint32_t    parent = 0;
        uint32_t    index = 0;
        std::string name;

        std::vector<glm::quat> rot;     // absolute transform matrix for animation
        std::vector<glm::vec3> trans;   // vec.size() == num_frames
    };

    struct Mesh
    {
		struct Weight
		{
			uint32_t jointIndex = 0;
			float    w = 0.0f;
		};
		using Weights = std::array<Weight, 4>;   // max 4 bones per vertex
	    
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
	
	// animation
    std::vector<JointNode> skeleton;
    std::vector<AABB>      bboxes;        // size() == num_frames, one bbox at least
    uint32_t               num_frames = 0;
    float                  frame_rate = 0.0f;
};

class ModelSystem : public ISystem
{
public:
    static ModelComponent GetDefaultModelComponent() { return {}; }
	static bool LoadModel(std::string const & fname, ModelComponent & out_mdl);

    void        update(Registry & reg, float time_delta);
    std::string getName() const { return "ModelSystem"; }
};


#endif
