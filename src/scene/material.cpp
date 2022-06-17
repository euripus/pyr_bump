#include "material.h"


MaterialComponent MaterialSystem::GetDefaultMaterialComponent()
{
	static const uint8_t texData[] = {128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255, 255, 192, 128, 255,
                             128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255, 255, 192, 128, 255,
                             255, 192, 128, 255, 255, 192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255,
                             255, 192, 128, 255, 255, 192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255};

	MaterialComponent mat;

	m_ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	m_diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	m_specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_shininess = 0.0f;
	
	mat.m_diffuse.height = 4;
	mat.m_diffuse.width  = 4;
	mat.m_diffuse.type   = ImageData::PixelType::pt_rgba;
	mat.m_diffuse.data = std::make_unique<uint8_t[]>(4 * 4 * 4);
	std::memcpy(mat.m_diffuse.data.get(), texData, 4 * 4 * 4);
	
	mat.m_bump.height = 4;
	mat.m_bump.width  = 4;
	mat.m_bump.type   = ImageData::PixelType::pt_rgba;
	mat.m_bump.data = std::make_unique<uint8_t[]>(4 * 4 * 4);
	std::memcpy(mat.m_bump.data.get(), texData, 4 * 4 * 4);
	
	return mat;
}