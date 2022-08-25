#include <fstream>
#include <sstream>

#include "scene.h"
#include "model.h"

void JointSystem::update(float time_delta)
{
    for(auto ent : m_reg.view<ModelComponent, CurrentAnimSequence>())
    {
        auto const & seq = m_reg.get<CurrentAnimSequence>(ent);
        auto const & mdl = m_reg.get<ModelComponent>(ent);

        auto const & cur_animation = mdl.animations[seq.id];
        // double control_ime = controller.GetControlTime(time));
        auto frame = getCurrentFrame(time_delta, cur_animation);
        updateModelJoints(mdl, frame);
        updateMdlBbox(ent, frame);
    }
}

JointsTransform JointSystem::getCurrentFrame(double time, AnimSequence const & seq) const
{
    float           frame_delta(0.0f);
    uint32_t        prev_frame = 0;
    uint32_t        next_frame = 0;
    JointsTransform cur_frame;

    // double control_ime = controller.GetControlTime(time));
    prev_frame = glm::floor(time * seq.frame_rate);
    next_frame = prev_frame + 1;
    if(next_frame == seq.frames.size() + 1)
        next_frame = 0;

    frame_delta    = time * seq.frame_rate - prev_frame;
    cur_frame.bbox = {
        glm::mix(seq.frames[prev_frame].bbox.min(), seq.frames[next_frame].bbox.min(), frame_delta),
        glm::mix(seq.frames[prev_frame].bbox.max(), seq.frames[next_frame].bbox.max(), frame_delta)};
    for(unsigned int i = 0; i < seq.frames[0].rot.size(); i++)
    {
        cur_frame.rot.push_back(glm::normalize(
            glm::slerp(seq.frames[prev_frame].rot[i], seq.frames[next_frame].rot[i], frame_delta)));
        cur_frame.trans.push_back(
            glm::mix(seq.frames[prev_frame].trans[i], seq.frames[next_frame].trans[i], frame_delta));
    }

    return cur_frame;
}

void JointSystem::updateModelJoints(ModelComponent const & mdl, JointsTransform const & frame) const
{
    for(uint32_t i = 0; i < frame.rot.size(); ++i)
    {
        auto      joint_ent = mdl.bone_id_to_entity[i];
        glm::mat4 mt        = glm::mat4_cast(frame.rot[i]);
        mt                  = glm::column(mt, 3, glm::vec4(frame.trans[i], 1.0f));

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

bool ModelSystem::LoadModel(std::string const & fname, ModelComponent & out_mdl,
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
        else if(line.substr(0, 3) == "vtx")
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
            s >> w.jointIndex >> w.w;
            w.jointIndex--;
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
        else if(line.substr(0, 3) == "jnt")
        {
            std::istringstream s(line.substr(3));
            int32_t            bone_id{}, parent_id{};
            std::string        bone_name;
            ParsedJoint        joint;

            s >> bone_id >> parent_id >> bone_name;

            assert(bone_id > 0);
            assert(bone_id > parent_id);

            joint.index  = --bone_id;
            joint.parent = --parent_id;
            joint.name   = bone_name;

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
