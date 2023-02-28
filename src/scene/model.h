#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>

#include "AABB.h"
#include "sceneentitybuilder.h"
#include "src/scene/scenecmp.h"
#include "src/utils/controller.h"

struct Mesh
{
    struct Weight
    {
        uint32_t joint_index = 0;
        float    w           = 0.0f;
    };

    // dynamic data initial
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> normal;
    std::vector<glm::vec3> tangent;
    std::vector<glm::vec3> bitangent;
    // dynamic data transformed
    std::vector<glm::vec3> frame_pos;
    std::vector<glm::vec3> frame_normal;
    std::vector<glm::vec3> frame_tangent;
    std::vector<glm::vec3> frame_bitangent;
    // static data
    std::vector<std::pair<uint32_t, uint32_t>>
                           weight_indxs;   // start and end indicies for vertex in weights_vec
    std::vector<Weight>    weights;
    std::vector<glm::vec2> tex_coords;
    std::vector<uint32_t>  indexes;

    evnt::AABB bbox;
};

struct JointComponent
{
    int32_t     index = 0;   // -1 for root
    std::string name;
    glm::mat4   inv_bind;
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
    Controller                   controller;
};

struct CurrentAnimSequence
{
    uint32_t id = 0;
};

struct ParsedJoint
{
    int32_t     index  = 0;
    int32_t     parent = 0;
    std::string name;
    glm::mat4   inv_bind;
};

// Model consist of next sequence of components:
//      [scene] [material] [model] [current_anim_sequence] - animated
//      [scene] [material] [model]                         - static
//      update joints => update scene => update models => render upload
struct ModelComponent
{
    std::vector<Mesh>         meshes;
    std::vector<Entity>       joint_id_to_entity;   // skel
    std::vector<AnimSequence> animations;
    std::string               material_name;

    evnt::AABB base_bbox;
};

namespace Event
{
namespace Model
{
    struct VertexDataChanged
    {};

    struct UploadBuffer
    {};

    struct UploadTexture
    {};

    struct UnloadBuffer
    {};

    struct UnloadTexture
    {};

    struct CreateModel
    {
        Entity      parent = null_entity_id;
        std::string mesh_name;
        std::string anim_name;
        std::string material_name;
        glm::mat4   rel_transform;
    };

    struct LoadModel
    {
        SceneSystem * scene = nullptr;
        std::string   mesh_name;
        std::string   anim_name;
        std::string   material_name;
    };

    struct DestroyModel
    {};
}   // namespace Model
}   // namespace Event

// Joint consist of
//      [scene] [joint_component]
//      joint.scene.matrix_rel = cur_anim_sequence[joint_node.idx].matrix
class JointSystem : public ISystem
{
public:
    JointSystem(Registry & reg) : ISystem(reg) {}

    void        update(double time = 1.0) override;
    std::string getName() const override { return "JointSystem"; }

private:
    JointsTransform getCurrentFrame(double time, AnimSequence const & seq) const;
    void            updateModelJoints(Entity mdl, JointsTransform const & frame) const;
    void            updateMdlBbox(Entity mdl, JointsTransform const & frame) const;
};

class ModelSystem : public ISystem
{
public:
    static ModelComponent GetDefaultModelComponent() { return {}; }
    static bool           LoadMesh(std::string const & fname, ModelComponent & out_mdl,
                                   std::vector<ParsedJoint> & joints);
    static bool           LoadAnim(std::string const & fname, ModelComponent & out_mdl);

    ModelSystem(Registry & reg) : ISystem(reg) {}
    // bool        init() override { return true; }
    void        update(double time = 1.0) override;
    void        postUpdate() override;   // clear tag structures
    std::string getName() const override { return "ModelSystem"; }

    void loadModel(Entity model_ent, SceneSystem & scene_sys, std::string const & fname,
                   std::string const & anim_fname, std::string const & mat_fname) const;
    void deleteModel(Entity model_id) const;

    std::optional<Entity> getJointIdFromName(Entity model_id, std::string const & bone_name);
};

#endif
