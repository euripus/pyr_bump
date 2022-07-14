#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AABB.h"
#include "sceneentitybuilder.h"

struct Mesh
{
	struct Weight
	{
		uint32_t jointIndex = 0;
		float    w          = 0.0f;
	};

	// dynamic data
	std::vector<glm::vec3> pos;
	std::vector<glm::vec3> normal;
	std::vector<glm::vec3> tangent;
	std::vector<glm::vec3> bitangent;
	// static data
	std::vector<std::pair<uint32_t, uint32_t>>
						   weight_indxs;   // start and end indicies for vertex in weights_vec
	std::vector<Weight>    weights;
	std::vector<glm::vec2> tex_coords;
	std::vector<uint32_t>  indexes;

	evnt::AABB bbox;
};

struct JointNode
{
	uint32_t    index  = 0;
	std::string name;
	Entity      model_id;
};

struct JointsTransform
{
	evnt::AABB bbox;

	std::vector<glm::quat> rot;     // absolute transform matrix for animation
	std::vector<glm::vec3> trans;   // vec.size() == num_bones
};

struct AnimSequence
{
    std::vector<JointsTransform> frames;
    float                        frame_rate = 0.0f;
};

struct CurrentFrame
{
	JointsTransform frame; 
};

struct CurrentAnimSequence
{
	uint32_t id = 0;
};

// Model consist of next sequence of components:
// 		[scene] [material] [model] [current_anim_sequence] [cur_frame]
struct ModelComponent
{
    std::vector<Mesh>         meshes;
	Entity                    root_bone = null_entity_id;
	std::vector<Entity>       bone_id_to_entity;
	std::vector<AnimSequence> animations;
	
	evnt::AABB base_bbox;
};

// Joint consist of
// 		[scene] [joint_node]
//		joint.scene.matrix_rel = cur_anim_sequence[joint_node.idx].matrix

class JointSystem  : public ISystem
{
	// must be called before scene.update()
	// 		cur_frame = getFrameInAnimSequence()
	// 		transform_bones(cur_frame)
};

class ModelSystem  : public ISystem
{
public:
    static ModelComponent GetDefaultModelComponent() { return {}; }
    static bool           LoadModel(std::string const & fname, ModelComponent & out_mdl);
    static bool           LoadAnim(std::string const & fname, ModelComponent & out_mdl);
	
	bool        init() { return true; }
	// must be called after scene.update()
	// for(msh : meshes)
	// 		for(vtx : msh)
	//  		mat4 n_mat;
	//			for(ws : we)
	//				joint = getJoint(ws.joint_index)
	//				mat = mat * ws.w(model_inv * joint.abs)
	//			new_vtx = mat * vtx
    void        update(float time_delta = 1.0f);
    void        postUpdate(); // clear CurrentFrame
    std::string getName() const;
	
	CurrentFrame getFrameInAnimSequence(Entity model_id, float time);
};

#endif
