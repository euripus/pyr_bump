#include "glmesh.h"
#include <GL/glew.h>

bool LoadGlMeshComponent(std::string const & mesh_fname, GlMeshComponent & out_mesh)
{
	// Load mesh
	MeshData geom;
    if(!LoadMesh(mesh_fname, geom))
        return false;
    // Generate buffers
    glGenBuffers(1, &out_mesh.m_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.m_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, geom.pos.size() * 3 * sizeof(float), geom.pos.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &out_mesh.m_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.m_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, geom.normals.size() * 3 * sizeof(float), geom.normals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &out_mesh.m_tangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.m_tangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, geom.tangent.size() * 3 * sizeof(float), geom.tangent.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &out_mesh.m_bitangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.m_bitangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, geom.bitangent.size() * 3 * sizeof(float), geom.bitangent.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &out_mesh.m_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.m_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, geom.tex.size() * 2 * sizeof(float), geom.tex.data(), GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &out_mesh.m_elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh.m_elementbuffer);
    out_mesh.m_indices_size = static_cast<GLsizei>(geom.indicies.size());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(out_mesh.m_indices_size) * sizeof(unsigned short),
                 geom.indicies.data(), GL_STATIC_DRAW);
	
	return true;
}

void UnloadFromGL(GlMeshComponent & mesh)
{
	glDeleteBuffers(1, &mesh.m_vertexbuffer);
	glDeleteBuffers(1, &mesh.m_normalbuffer);
	glDeleteBuffers(1, &mesh.m_tangentbuffer);
	glDeleteBuffers(1, &mesh.m_bitangentbuffer);
	glDeleteBuffers(1, &mesh.m_uvbuffer);
	glDeleteBuffers(1, &mesh.m_elementbuffer);

	std::swap(mesh, GlMeshComponent{});
}