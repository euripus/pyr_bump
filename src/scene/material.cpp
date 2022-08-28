#include "material.h"
#include <cstring>

MaterialComponent MaterialSystem::GetDefaultMaterialComponent()
{
    static const uint8_t texData[] = {128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255, 255,
                                      192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255, 255, 192,
                                      128, 255, 255, 192, 128, 255, 255, 192, 128, 255, 255, 192, 128,
                                      255, 128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255,
                                      255, 192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255};

    MaterialComponent mat;

    mat.m_ambient   = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    mat.m_diffuse   = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    mat.m_specular  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_emission  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_shininess = 0.0f;

    mat.m_base_tex_id = 0;
    mat.m_diff_tex_id = 0;

    mat.m_diff.height = 4;
    mat.m_diff.width  = 4;
    mat.m_diff.type   = tex::ImageData::PixelType::pt_rgba;
    mat.m_diff.data   = std::make_unique<uint8_t[]>(4 * 4 * 4);
    std::memcpy(mat.m_diff.data.get(), texData, 4 * 4 * 4);

    mat.m_bump.height = 4;
    mat.m_bump.width  = 4;
    mat.m_bump.type   = tex::ImageData::PixelType::pt_rgba;
    mat.m_bump.data   = std::make_unique<uint8_t[]>(4 * 4 * 4);
    std::memcpy(mat.m_bump.data.get(), texData, 4 * 4 * 4);

    return mat;
}

bool MaterialSystem::LoadTGA(MaterialComponent & mat, std::string const & base_fname,
                             std::string const & bump_fname)
{
    if(!tex::ReadTGA(base_fname, mat.m_diff))
        return false;

    if(!bump_fname.empty())
        if(!tex::ReadTGA(bump_fname, mat.m_bump))
            return false;

    return true;
}
