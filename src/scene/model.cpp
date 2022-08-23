#include <fstream>
#include <sstream>

#include "model.h"

bool ModelSystem::LoadModel(std::string const & fname, evnt::SceneSystem & sys, ModelComponent & out_mdl)
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
            cur_mesh->weights.push_back(w);
        }
        else if(line.substr(0, 4) == "bones")
        {
            std::istringstream s(line.substr(4));
            uint32_t           num;

            s >> num;
            out_mdl.bone_id_to_entity.clear();
            out_mdl.bone_id_to_entity.resize(num);
        }
        else if(line.substr(0, 3) == "jnt")
        {
            std::istringstream s(line.substr(3));
            int32_t            bone_id{}, parent_id{};
            std::string        bone_name;

            s >> bone_id >> parent_id >> bone_name;

            assert(bone_id > 0);
            assert(bone_id > parent_id);

            auto joint_entity = SceneEntityBuilder::BuildEntity(sys.getRegistry(), joint_flags);

            auto & jc = sys.getRegistry().get<JointComponent>(joint_entity);
            jc.index  = bone_id;
            jc.name   = bone_name;

            out_mdl.bone_id_to_entity[bone_id - 1] = joint_entity;
            auto parent_entity                     = out_mdl.bone_id_to_entity[parent_id - 1];

            sys.addNode(joint_entity, parent_entity);
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
