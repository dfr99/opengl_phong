#version 130

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
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
in vec2 TexCoords;

uniform Material material;
uniform Light light;
uniform vec3 view_pos;

void main() {

  // Ambiente
  vec3 ambient = light.ambient * material.ambient;

  // Difusión
  vec3 light_dir = normalize(light.position - frag_3Dpos);
  float diff = max(dot(vs_normal, light_dir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse);

  // Especular
  vec3 view_dir = normalize(view_pos - frag_3Dpos);
  vec3 reflect_dir = reflect(-light_dir, vs_normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * material.specular);


  vec3 result = ambient + diffuse + specular;
  frag_col = vec4(result, 1.0);
}
