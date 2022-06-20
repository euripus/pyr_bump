#ifndef GLMESH_H
#define GLMESH_H

#include <string>
#include "../res/meshdata.h"
#include <GL/glew.h>

struct GlMeshComponent
{
    GLuint m_vertexbuffer    = 0;
    GLuint m_uvbuffer        = 0;
    GLuint m_normalbuffer    = 0;
    GLuint m_tangentbuffer   = 0;
    GLuint m_bitangentbuffer = 0;
    GLuint m_elementbuffer   = 0;

    GLsizei m_indices_size = 0;
};

bool LoadGlMeshComponent(std::string const & mesh_fname, GlMeshComponent & out_mesh);

void UnloadFromGL(GlMeshComponent & mesh);

#endif   // MESHDATA_H
