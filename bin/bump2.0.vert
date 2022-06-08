#version 120

uniform vec3 light_pos_world;
uniform vec3 cam_pos_world;

varying vec2 tex;
varying vec3 view_dir_tang;
varying vec3 light_dir_tang;

attribute vec3 rm_bitang;
attribute vec3 rm_tangent;

void main(void)
{
    gl_Position = ftransform();
    tex         = gl_MultiTexCoord0.xy;

    vec4 vpos_cam = gl_ModelViewMatrix * gl_Vertex;

    // In camera space, the camera is at the origin (0,0,0).
    // vec3 view_dir = cam_pos_world - (Model * gl_Vertex).xyz;
    vec3 view_dir  = vec3(0, 0, 0) - vpos_cam.xyz;
    // Vector that goes from the vertex to the light, in camera space.
    // vec4 light_pos_cam = View * LightModelMatrix * vec4(light_pos_model, 1.0);
    vec3 light_dir = light_pos_world - vpos_cam.xyz;

    // gl_NormalMatrix = transpose(inverse(mat3(modelview)));
    vec3 norm_tang  = gl_NormalMatrix * gl_Normal;
    vec3 bitan_tang = gl_NormalMatrix * rm_bitang;
    vec3 tan_tang   = gl_NormalMatrix * rm_tangent;

    view_dir_tang.x = dot(tan_tang, view_dir);
    view_dir_tang.y = dot(bitan_tang, view_dir);
    view_dir_tang.z = dot(norm_tang, view_dir);

    light_dir_tang.x = dot(tan_tang, light_dir);
    light_dir_tang.y = dot(bitan_tang, light_dir);
    light_dir_tang.z = dot(norm_tang, light_dir);
}
