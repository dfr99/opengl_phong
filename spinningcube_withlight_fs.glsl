#version 130

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

out vec4 frag_col;

in vec3 frag_3Dpos;
in vec3 vs_normal;
in vec2 vs_tex_coord;

uniform Material material;
uniform Light light;
uniform Light light2;
uniform vec3 view_pos;

void main() {

  // Ambiente -> light1
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_tex_coord));
  
  // Ambiente -> light2
  vec3 ambient2 = light2.ambient * vec3(texture(material.diffuse, vs_tex_coord));

  // Difusión -> light1
  vec3 light_dir = normalize(light.position - frag_3Dpos);
  float diff = max(dot(vs_normal, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff *
                 vec3(texture(material.diffuse, vs_tex_coord));

  // Difusión -> light2
  vec3 light2_dir = normalize(light2.position - frag_3Dpos);
  float diff2 = max(dot(vs_normal, light2_dir), 0.0);
  vec3 diffuse2 = light2.diffuse * diff2 *
                  vec3(texture(material.diffuse, vs_tex_coord));

  vec3 view_dir = normalize(view_pos - frag_3Dpos);

  // Especular -> light1
  vec3 reflect_dir = reflect(-light_dir, vs_normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * vec3(texture(material.specular, vs_tex_coord)));

  // Especular -> light2
  vec3 reflect2_dir = reflect(-light2_dir, vs_normal);
  float spec2 = pow(max(dot(view_dir, reflect2_dir), 0.0), material.shininess);
  vec3 specular2 = light2.specular * (spec2 * vec3(texture(material.specular, vs_tex_coord)));

  vec3 result = ambient + ambient2 + diffuse + diffuse2 + specular + specular2;
  frag_col = vec4(result, 1.0);
}
