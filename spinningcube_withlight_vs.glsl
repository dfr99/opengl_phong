#version 130

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_tex;

out vec3 frag_3Dpos;
out vec3 vs_normal;
out vec2 vs_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_to_world;

void main() {

  frag_3Dpos = vec3(model * vec4(v_pos, 1.0));
  vs_normal = normalize(normal_to_world * v_normal);

  gl_Position = projection * view * model * vec4(v_pos, 1.0f);
  vs_tex_coord = v_tex;
}
