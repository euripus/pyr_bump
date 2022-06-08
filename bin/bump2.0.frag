#version 120

uniform vec4  ambient_col;
uniform vec4  specular_col;
uniform float specular_pow;

uniform sampler2D base_map;
uniform sampler2D bump_map;

varying vec2 tex;
varying vec3 view_dir_tang;
varying vec3 light_dir_tang;

void main(void)
{
    vec3  light_dir_tang_norm = normalize(light_dir_tang);
    vec3  bump_nor            = normalize((texture2D(bump_map, tex).xyz * 2.0) - 1.0);
    float n_dot_l             = dot(bump_nor, light_dir_tang_norm);

    vec3  reflection         = normalize(((2.0 * bump_nor) * n_dot_l) - light_dir_tang_norm);
    vec3  view_dir_tang_norm = normalize(view_dir_tang);
    float r_dot_v            = max(0.0, dot(reflection, view_dir_tang_norm));

    vec4 base_col = texture2D(base_map, tex);

    vec4 total_ambient  = ambient_col * base_col;
    vec4 total_diffuse  = n_dot_l * base_col;
    vec4 total_specular = specular_col * (pow(r_dot_v, specular_pow));

    gl_FragColor = (total_ambient + total_diffuse + total_specular);
}
