#include <fstream>
#include <sstream>
#include <algorithm>
// glm::to_string
//#include <glm/gtx/string_cast.hpp>
//#include <iostream>

#include "scene.h"
#include "material.h"
#include "model.h"

void JointSystem::update(double time)
{
    for(auto ent : m_reg.view<ModelComponent, CurrentAnimSequence>())
    {
        auto const & seq = m_reg.get<CurrentAnimSequence>(ent);
        auto const & mdl = m_reg.get<ModelComponent>(ent);

        auto const & cur_animation = mdl.animations[seq.id];

        auto frame = getCurrentFrame(time, cur_animation);
        updateModelJoints(ent, frame);
        updateMdlBbox(ent, frame);
    }
}

JointsTransform JointSystem::getCurrentFrame(double time, AnimSequence const & frame_seq) const
{
    float           frame_delta = 0.0f;
    uint32_t        last_frame  = 0;
    uint32_t        next_frame  = 0;
    JointsTransform cur_frame;

    double control_time = frame_seq.controller.getControlTime(time);
    last_frame          = static_cast<uint32_t>(glm::floor(control_time * frame_seq.frame_rate));
    next_frame          = last_frame + 1;
    if(next_frame == frame_seq.frames.size())
        next_frame = 0;

    frame_delta    = static_cast<float>(control_time * frame_seq.frame_rate - last_frame);
    cur_frame.bbox = {glm::mix(frame_seq.frames[last_frame].bbox.min(),
                               frame_seq.frames[next_frame].bbox.min(), frame_delta),
                      glm::mix(frame_seq.frames[last_frame].bbox.max(),
                               frame_seq.frames[next_frame].bbox.max(), frame_delta)};
    for(uint32_t i = 0; i < frame_seq.frames[0].rot.size(); i++)
    {
        cur_frame.rot.push_back(glm::normalize(glm::slerp(frame_seq.frames[last_frame].rot[i],
                                                          frame_seq.frames[next_frame].rot[i], frame_delta)));
        cur_frame.trans.push_back(glm::mix(frame_seq.frames[last_frame].trans[i],
                                           frame_seq.frames[next_frame].trans[i], frame_delta));
    }

    return cur_frame;
}

void JointSystem::updateModelJoints(Entity ent, JointsTransform const & frame) const
{
    auto const & mdl = m_reg.get<ModelComponent>(ent);

    for(uint32_t i = 0; i < mdl.bone_id_to_entity.size(); ++i)
    {
        auto joint_ent = mdl.bone_id_to_entity[i];

        glm::mat4 mt = glm::mat4_cast(frame.rot[i]);
        mt           = glm::column(mt, 3, glm::vec4(frame.trans[i], 1.0f));

        evnt::TransformComponent transform{};
        transform.replase_local_matrix = true;
        transform.new_mat              = mt;
        m_reg.add_component<evnt::TransformComponent>(joint_ent, transform);
    }
}

void JointSystem::updateMdlBbox(Entity ent, JointsTransform const & frame) const
{
    auto & pos = m_reg.get<evnt::SceneComponent>(ent);
    auto & mdl = m_reg.get<ModelComponent>(ent);

    pos.bbox      = frame.bbox;
    mdl.base_bbox = frame.bbox;
}

bool ModelSystem::LoadMesh(std::string const & fname, ModelComponent & out_mdl,
                           std::vector<ParsedJoint> & joints)
{
    if(!out_mdl.meshes.empty())
        return false;   // out model not empty

    std::ifstream in(fname, std::ios::in);
    if(!in)
        return false;

    std::string line;
    Mesh *      cur_mesh = nullptr;
    while(std::getline(in, line))
    {
        if(line.substr(0, 6) == "meshes")
        {
            uint32_t           num_meshes;
            std::istringstream s(line.substr(6));
            s >> num_meshes;

            out_mdl.meshes.resize(num_meshes);
        }
        else if(line.substr(0, 4) == "mesh")
        {
            uint32_t           num_mesh;
            std::istringstream s(line.substr(4));
            s >> num_mesh;

            cur_mesh = &out_mdl.meshes[num_mesh];
        }
        else if(line.substr(0, 4) == "bbox")
        {
            std::istringstream s(line.substr(4));
            float              mnx(0), mny(0), mnz(0), mxx(0), mxy(0), mxz(0);
            s >> mnx >> mny >> mnz >> mxx >> mxy >> mxz;
            cur_mesh->bbox = evnt::AABB(mnx, mny, mnz, mxx, mxy, mxz);
        }
        else if(line.substr(0, 3) == "vps")
        {
            std::istringstream s(line.substr(3));
            glm::vec3          v;
            s >> v.x >> v.y >> v.z;
            cur_mesh->pos.push_back(v);
        }
        else if(line.substr(0, 3) == "vnr")
        {
            std::istringstream s(line.substr(3));
            glm::vec3          v;
            s >> v.x >> v.y >> v.z;
            cur_mesh->normal.push_back(glm::normalize(v));
        }
        else if(line.substr(0, 3) == "vtg")
        {
            std::istringstream s(line.substr(3));
            glm::vec3          v;
            s >> v.x >> v.y >> v.z;
            cur_mesh->tangent.push_back(glm::normalize(v));
        }
        else if(line.substr(0, 3) == "vbt")
        {
            std::istringstream s(line.substr(3));
            glm::vec3          v;
            s >> v.x >> v.y >> v.z;
            cur_mesh->bitangent.push_back(glm::normalize(v));
        }
        else if(line.substr(0, 3) == "tx0")
        {
            std::istringstream s(line.substr(3));
            glm::vec2          v;
            s >> v.x >> v.y;
            cur_mesh->tex_coords.push_back(v);
        }
        else if(line.substr(0, 3) == "fcx")
        {
            std::istringstream s(line.substr(3));
            uint32_t           v1(0), v2(0), v3(0);
            s >> v1 >> v2 >> v3;
            cur_mesh->indexes.push_back(v1);
            cur_mesh->indexes.push_back(v2);
            cur_mesh->indexes.push_back(v3);
        }
        else if(line.substr(0, 3) == "wgi")
        {
            std::istringstream s(line.substr(3));
            uint32_t           end_wght_ind = 0;

            s >> end_wght_ind;
            std::pair<uint32_t, uint32_t> wgh_ind;
            wgh_ind.second = end_wght_ind;
            if(!cur_mesh->weight_indxs.empty())
                wgh_ind.first = cur_mesh->weight_indxs.back().second;
            else
                wgh_ind.first = 0;

            cur_mesh->weight_indxs.push_back(wgh_ind);
        }
        else if(line.substr(0, 3) == "wgh")
        {
            std::istringstream s(line.substr(3));
            Mesh::Weight       w;
            s >> w.joint_index >> w.w;
            w.joint_index--;
            cur_mesh->weights.push_back(w);
        }
        else if(line.substr(0, 5) == "bones")
        {
            std::istringstream s(line.substr(5));
            uint32_t           num;

            s >> num;
            out_mdl.bone_id_to_entity.clear();
            out_mdl.bone_id_to_entity.resize(num, null_entity_id);
        }
        else if(line.substr(0, 8) == "material")
        {
            std::istringstream s(line.substr(8));
            std::string        name;

            s >> name;
            out_mdl.material_name = name;
        }
        else if(line.substr(0, 3) == "jnt")
        {
            std::istringstream s(line.substr(3));
            int32_t            bone_id{}, parent_id{};
            std::string        bone_name;
            float              qtx(0), qty(0), qtz(0), qtw(0), tr_x(0), tr_y(0), tr_z(0);
            ParsedJoint        joint;

            s >> bone_id >> parent_id >> bone_name >> qtx >> qty >> qtz >> qtw >> tr_x >> tr_y >> tr_z;

            assert(bone_id > 0);
            assert(bone_id > parent_id);

            joint.index  = --bone_id;
            joint.parent = --parent_id;
            joint.name   = bone_name;

            auto rot   = glm::quat(qtw, qtx, qty, qtz);
            auto trans = glm::vec3(tr_x, tr_y, tr_z);

            glm::mat4 mt   = glm::mat4_cast(rot);
            mt             = glm::column(mt, 3, glm::vec4(trans, 1.0f));
            joint.inv_bind = mt;

            joints.push_back(std::move(joint));
        }
    }
    in.close();

    evnt::AABB bbox;
    for(auto & msh : out_mdl.meshes)
    {
        bbox.expandBy(msh.bbox);
    }
    out_mdl.base_bbox = bbox;

    // check data correctness
    for(auto const & msh : out_mdl.meshes)
    {
        if(!(msh.pos.size() == msh.normal.size() && msh.pos.size() == msh.tangent.size()
             && msh.pos.size() == msh.bitangent.size() && msh.pos.size() == msh.tex_coords.size()))
            return false;
    }

    return true;
}

bool ModelSystem::LoadAnim(std::string const & fname, ModelComponent & out_mdl)
{
    std::ifstream in(fname, std::ios::in);
    if(!in)
        return false;

    std::string       line;
    JointsTransform * cur_frame = nullptr;
    AnimSequence      anm_sequence;
    uint32_t          jnt_ind   = 0;
    uint32_t          num_bones = 0;
    while(std::getline(in, line))
    {
        if(line.substr(0, 5) == "bones")
        {
            std::istringstream s(line.substr(5));
            s >> num_bones;
        }
        else if(line.substr(0, 6) == "frames")
        {
            uint32_t           num_frames;
            std::istringstream s(line.substr(6));
            s >> num_frames;

            anm_sequence.frames.resize(num_frames);
            for(auto & jnt : anm_sequence.frames)
            {
                jnt.rot.resize(num_bones);
                jnt.trans.resize(num_bones);
            }
        }
        else if(line.substr(0, 9) == "framerate")
        {
            float              framerate;
            std::istringstream s(line.substr(9));
            s >> framerate;

            anm_sequence.frame_rate = framerate;
        }
        else if(line.substr(0, 5) == "frame")
        {
            uint32_t           frame;
            std::istringstream s(line.substr(5));
            s >> frame;

            cur_frame = &anm_sequence.frames[frame];
            jnt_ind   = 0;
        }
        else if(line.substr(0, 4) == "bbox")
        {
            std::istringstream s(line.substr(4));
            float              mnx(0), mny(0), mnz(0), mxx(0), mxy(0), mxz(0);
            s >> mnx >> mny >> mnz >> mxx >> mxy >> mxz;

            cur_frame->bbox = evnt::AABB(mnx, mny, mnz, mxx, mxy, mxz);
        }
        else if(line.substr(0, 3) == "jtr")
        {
            std::istringstream s(line.substr(3));
            float              qtx(0), qty(0), qtz(0), qtw(0), tr_x(0), tr_y(0), tr_z(0);
            s >> qtx >> qty >> qtz >> qtw;
            s >> tr_x >> tr_y >> tr_z;

            cur_frame->rot[jnt_ind]   = glm::quat(qtw, qtx, qty, qtz);
            cur_frame->trans[jnt_ind] = glm::vec3(tr_x, tr_y, tr_z);
            jnt_ind++;
        }
    }
    in.close();

    anm_sequence.controller =
        Controller(Controller::RepeatType::RT_WRAP, 0.0,
                   static_cast<double>(anm_sequence.frames.size()) / anm_sequence.frame_rate);

    // check data correctness
    for(auto const & frm : anm_sequence.frames)
    {
        if(out_mdl.bone_id_to_entity.size() != frm.rot.size()
           || out_mdl.bone_id_to_entity.size() != frm.trans.size())
            return false;
    }

    out_mdl.animations.push_back(std::move(anm_sequence));
    return true;
}

void ModelSystem::update(double time)
{
    // update positions for animated meshes
    for(auto ent : m_reg.view<ModelComponent, CurrentAnimSequence>())
    {
        auto &       geom = m_reg.get<ModelComponent>(ent);
        auto const & scn  = m_reg.get<evnt::SceneComponent>(ent);

        glm::mat4 inverted_model = glm::inverse(scn.abs);

        for(auto & msh : geom.meshes)
        {
            msh.frame_pos.clear();
            msh.frame_normal.clear();
            msh.frame_tangent.clear();
            msh.frame_bitangent.clear();

            for(uint32_t n = 0; n < msh.pos.size(); ++n)
            {
                glm::mat4 vert_mat(0.0f);
                for(uint32_t j = msh.weight_indxs[n].first; j < msh.weight_indxs[n].second; ++j)
                {
                    auto         joint_ent = geom.bone_id_to_entity[msh.weights[j].joint_index];
                    auto const & joint_scn = m_reg.get<evnt::SceneComponent>(joint_ent);
                    auto const & jont_cmp  = m_reg.get<JointComponent>(joint_ent);

                    vert_mat += inverted_model * joint_scn.abs * jont_cmp.inv_bind * msh.weights[j].w;
                }
                glm::mat3 norm_mat = glm::mat3(vert_mat);

                glm::vec4 n_pos   = vert_mat * glm::vec4(msh.pos[n], 1.0);
                glm::vec3 n_norm  = norm_mat * msh.normal[n];
                glm::vec3 n_tang  = norm_mat * msh.tangent[n];
                glm::vec3 n_bitan = norm_mat * msh.bitangent[n];

                msh.frame_pos.push_back(glm::vec3(n_pos));
                msh.frame_normal.push_back(n_norm);
                msh.frame_tangent.push_back(n_tang);
                msh.frame_bitangent.push_back(n_bitan);
            }
        }
        // mark for render for update buffers data
        m_reg.add_component<VertexDataChanged>(ent);
    }
}

void ModelSystem::postUpdate()
{
    m_reg.reset<VertexDataChanged>();
}

Entity ModelSystem::loadModel(evnt::SceneSystem & scene_sys, std::string const & fname,
                              std::string const & anim_fname) const
{
    auto model_ent = SceneEntityBuilder::BuildEntity(m_reg, obj_flags);

    auto & geom = m_reg.get<ModelComponent>(model_ent);
    auto & mat  = m_reg.get<MaterialComponent>(model_ent);
    auto & scn  = m_reg.get<evnt::SceneComponent>(model_ent);

    // Load mesh
    std::vector<ParsedJoint> joints;
    if(!ModelSystem::LoadMesh(fname, geom, joints))
        throw std::runtime_error{"Failed to load mesh"};

    // Load the textures
    if(!MaterialSystem::LoadTGA(mat, geom.material_name, {}))
        throw std::runtime_error{"Failed to load texture"};

    // set AABB
    scn.bbox = geom.base_bbox;

    // if we have skeleton
    if(!joints.empty())
    {
        // and animation
        if(ModelSystem::LoadAnim(anim_fname, geom))
        {
            // add joints to the scene
            for(auto & jnt : joints)
            {
                auto   joint_ent = SceneEntityBuilder::BuildEntity(m_reg, joint_flags);
                auto & jnt_cmp   = m_reg.get<JointComponent>(joint_ent);

                geom.bone_id_to_entity[jnt.index] = joint_ent;
                Entity parent_ent                 = model_ent;
                if(jnt.parent != -1)
                    parent_ent = geom.bone_id_to_entity[jnt.parent];

                jnt_cmp.index    = jnt.index;
                jnt_cmp.name     = jnt.name;
                jnt_cmp.inv_bind = jnt.inv_bind;

                scene_sys.addNode(joint_ent, parent_ent);
            }
            m_reg.assign<CurrentAnimSequence>(model_ent);
        }
    }

    return model_ent;
}
