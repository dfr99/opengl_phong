// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// GLM library to deal with matrix operations
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "textfile_ALT.h"

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(double, GLuint *vao, GLuint *pyramidVao);
void obtenerNormales(GLfloat * normales,const GLfloat vertices[]);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertext Array Object to set input data
GLint model_location, view_location, proj_location, normal_location; // Uniforms for transformation matrices
GLint light_position_location, light_ambient_location, light_diffuse_location, light_specular_location; // Uniforms for light1 data
GLint light2_position_location, light2_ambient_location, light2_diffuse_location, light2_specular_location; // Uniforms for light2 data
GLint material_ambient_location, material_diffuse_location, material_specular_location, material_shininess_location; // Uniforms for material matrices
GLint camera_pos_location;

// Shader names
const char *vertexFileName = "spinningcube_withlight_vs.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);

// Lighting (light1)
glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);

// Lighting (light2)
glm::vec3 light2_pos(1.0f, 1.0f, 2.0f);
glm::vec3 light2_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light2_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light2_specular(1.0f, 1.0f, 1.0f);

// Material
glm::vec3 material_ambient(1.0f, 0.5f, 0.31f);
glm::vec3 material_diffuse(1.0f, 0.5f, 0.31f);
glm::vec3 material_specular(0.5f, 0.5f, 0.5f);
const GLfloat material_shininess = 32.0f;

int main() {
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "My spinning cube", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* vendor = glGetString(GL_VENDOR); // get vendor string
  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* glversion = glGetString(GL_VERSION); // version as a string
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", glversion);
  printf("GLSL version supported %s\n", glslversion);
  printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

  // Enable Depth test: only draw onto a pixel if fragment closer to viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // set a smaller value as "closer"

  // Vertex Shader
  char* vertex_shader = textFileRead(vertexFileName);

  // Fragment Shader
  char* fragment_shader = textFileRead(fragmentFileName);

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  free(vertex_shader);
  glCompileShader(vs);

  int  success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  free(fragment_shader);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glValidateProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

    return(1);
  }

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  const GLfloat vertex_positions[] = {
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f, -0.25f, // 0
     0.25f, -0.25f, -0.25f, // 2

     0.25f,  0.25f, -0.25f, // 3
     0.25f, -0.25f, -0.25f, // 2
    -0.25f,  0.25f, -0.25f, // 0

     0.25f, -0.25f, -0.25f, // 2
     0.25f,  0.25f, -0.25f, // 3
     0.25f, -0.25f,  0.25f, // 5

     0.25f,  0.25f,  0.25f, // 4
     0.25f, -0.25f,  0.25f, // 5
     0.25f,  0.25f, -0.25f, // 3

     0.25f, -0.25f,  0.25f, // 5
     0.25f,  0.25f,  0.25f, // 4
    -0.25f, -0.25f,  0.25f, // 6

    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f,  0.25f, // 6
     0.25f,  0.25f,  0.25f, // 4

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f,  0.25f, // 7

     0.25f, -0.25f, -0.25f, // 2
     0.25f, -0.25f,  0.25f, // 5
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f, -0.25f, -0.25f, // 1
     0.25f, -0.25f,  0.25f, // 5

     0.25f,  0.25f,  0.25f, // 4
     0.25f,  0.25f, -0.25f, // 3
    -0.25f,  0.25f,  0.25f, // 7

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f,  0.25f,  0.25f, // 7
     0.25f,  0.25f, -0.25f  // 3
  };

  const float pyramidScaleFactor = 0.3;

  const GLfloat pyramidVertices[] = {
    // Front face
    0.0f, 1.0f, 0.0f,      // Vertex 0
    -1.0f, -1.0f, 1.0f,    // Vertex 1
    1.0f, -1.0f, 1.0f,     // Vertex 2

    // Right face
    0.0f, 1.0f, 0.0f,      // Vertex 3
    1.0f, -1.0f, 1.0f,     // Vertex 4
    1.0f, -1.0f, -1.0f,    // Vertex 5

    // Back face
    0.0f, 1.0f, 0.0f,      // Vertex 6
    1.0f, -1.0f, -1.0f,    // Vertex 7
    -1.0f, -1.0f, -1.0f,   // Vertex 8

    // Left face
    0.0f, 1.0f, 0.0f,      // Vertex 9
    -1.0f, -1.0f, -1.0f,   // Vertex 10
    -1.0f, -1.0f, 1.0f,    // Vertex 11

    // Bottom face
    -1.0f, -1.0f, 1.0f,    // Vertex 12
    1.0f, -1.0f, 1.0f,     // Vertex 13
    1.0f, -1.0f, -1.0f,    // Vertex 14
    -1.0f, -1.0f, -1.0f,   // Vertex 15
  };

  const GLuint pyramidIndices[] = {
    0, 1, 2,    // Front face
    3, 4, 5,    // Right face
    6, 7, 8,    // Back face
    9, 10, 11,  // Left face
    12, 13, 14, // Bottom face
    12, 14, 15, // Bottom face
  };

  GLfloat normales[sizeof(float)* 108] = {};

// Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // 1: vertex normals (x, y, z)
  obtenerNormales(normales, vertex_positions);
  GLuint normalesBuffer = 0;
  glGenBuffers(1, &normalesBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalesBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normales), normales, GL_STATIC_DRAW);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // note that we update the lamp's position attribute's stride to reflect the updated buffer data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 1);

  // Unbind vao
  glBindVertexArray(0);
  glBindVertexArray(1);

  // Pyramid VAO and VBO
  GLuint pyramidVao, pyramidVbo, pyramidIbo;
  glGenVertexArrays(1, &pyramidVao);
  glBindVertexArray(pyramidVao);

  glGenBuffers(1, &pyramidVbo);
  glBindBuffer(GL_ARRAY_BUFFER, pyramidVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

  glGenBuffers(1, &pyramidIbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidIbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

  // Set the vertex attribute pointers for the pyramid
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // Uniforms
  
  // - Model matrix
  model_location = glGetUniformLocation(shader_program, "model");
  
  // - View matrix
  view_location = glGetUniformLocation(shader_program, "view");

  // - Projection matrix
  proj_location = glGetUniformLocation(shader_program, "projection");

  // - Normal matrix: normal vectors from local to world coordinates
  normal_location = glGetUniformLocation(shader_program, "normal_to_world");
  
  // - Camera position
  camera_pos_location = glGetUniformLocation(shader_program, "view_pos");
  
  // - Light data (light1)
  light_position_location = glGetUniformLocation(shader_program, "light.position"); 
  light_ambient_location = glGetUniformLocation(shader_program, "light.ambient"); 
  light_diffuse_location = glGetUniformLocation(shader_program, "light.diffuse"); 
  light_specular_location = glGetUniformLocation(shader_program, "light.specular");
  
  // - Light data (light2)
  light2_position_location = glGetUniformLocation(shader_program, "light2.position");
  light2_ambient_location = glGetUniformLocation(shader_program, "light2.ambient");
  light2_diffuse_location = glGetUniformLocation(shader_program, "light2.diffuse");
  light_specular_location = glGetUniformLocation(shader_program, "light2.specular");

  // - Material data
  material_ambient_location = glGetUniformLocation(shader_program, "material.ambient"); 
  material_diffuse_location = glGetUniformLocation(shader_program, "material.diffuse"); 
  material_specular_location = glGetUniformLocation(shader_program, "material.specular"); 
  material_shininess_location = glGetUniformLocation(shader_program, "material.shininess");

// Render loop
  while(!glfwWindowShouldClose(window)) {

    processInput(window);

    render(glfwGetTime(), &vao, &pyramidVao);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime, GLuint *vao, GLuint *pyramidVao) {
  float f = (float)currentTime * 0.3f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(*vao);

  glm::mat4 model_matrix, view_matrix, proj_matrix, pyramid_matrix;
  glm::mat3 normal_matrix;

  model_matrix = glm::mat4(1.f);
  view_matrix = glm::lookAt(                 camera_pos,  // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 1.0f, 0.0f)); // up

  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 30.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));

  model_matrix = glm::rotate(model_matrix,
                            glm::radians((float)currentTime * 81.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));

  // Projection
  proj_matrix = glm::perspective(glm::radians(50.0f),
                                 (float) gl_width / (float) gl_height,
                                 0.1f, 1000.0f);
  
  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
  
  // Normal matrix: normal vectors to world coordinates
  normal_matrix = glm::inverseTranspose(glm::mat3(model_matrix));
  glUniformMatrix3fv(normal_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glUniform3fv(light_position_location, 1, glm::value_ptr(light_pos));
  glUniform3fv(light_ambient_location, 1, glm::value_ptr(light_ambient));
  glUniform3fv(light_diffuse_location, 1, glm::value_ptr(light_diffuse));
  glUniform3fv(light_specular_location, 1, glm::value_ptr(light_specular));

  glUniform3fv(light2_position_location, 1, glm::value_ptr(light2_pos));
  glUniform3fv(light2_ambient_location, 1, glm::value_ptr(light2_ambient));
  glUniform3fv(light2_diffuse_location, 1, glm::value_ptr(light2_diffuse));
  glUniform3fv(light2_specular_location, 1, glm::value_ptr(light2_specular));

  glUniform3fv(material_ambient_location, 1, glm::value_ptr(material_ambient));
  glUniform3fv(material_diffuse_location, 1, glm::value_ptr(material_diffuse));
  glUniform3fv(material_specular_location, 1, glm::value_ptr(material_specular));
  glUniform1f(material_shininess_location, material_shininess);

  glUniform3fv(camera_pos_location, 1, glm::value_ptr(camera_pos));

  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);

  // Draw the pyramid
  model_matrix = glm::mat4(1.f);
  glm::vec3 pyramid_pos(0.6f,0.0f,0.0f);

  view_matrix = glm::lookAt(                 camera_pos,  // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 1.0f, 0.0f)); // up

  model_matrix = glm::rotate(model_matrix,
                             glm::radians((float)currentTime * 30.0f),
                             glm::vec3(0.0f, 1.0f, 0.0f));

  model_matrix = glm::rotate(model_matrix,
                             glm::radians((float)currentTime * 81.0f),
                             glm::vec3(1.0f, 0.0f, 0.0f));

  model_matrix = glm::translate(model_matrix, pyramid_pos);
  model_matrix = glm::scale(model_matrix, glm::vec3(0.125f));

  // Projection
  proj_matrix = glm::perspective(glm::radians(50.0f),
                                 (float) gl_width / (float) gl_height,
                                 0.1f, 1000.0f);
  
  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
  
  // Normal matrix: normal vectors to world coordinates
  normal_matrix = glm::inverseTranspose(glm::mat3(model_matrix));
  glUniformMatrix3fv(normal_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glBindVertexArray(*pyramidVao);
  glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}

void obtenerNormales(GLfloat * normales,const GLfloat vertices[]){

  for(int i = 0; i < 108; i+= 9){

    GLfloat v1[] = {vertices[i], vertices[i+1], vertices[i+2]};
    GLfloat v2[] = {vertices[i+3], vertices[i+4], vertices[i+5]};
    GLfloat v3[] = {vertices[i+6], vertices[i+7], vertices[i+8]};

    GLfloat U[] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
    GLfloat V[] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};

    GLfloat x = U[1] * V[2] - U[2] * V[1];
    GLfloat y = U[2] * V[0] - U[0] * V[2];
    GLfloat z = U[0] * V[1] - U[1] * V[0];
    //printf("[%f, ", x);
    //printf("%f, ", y);
    //printf("%f] \n", z);
    
    normales[i] = x;      normales[i+3] = x; normales[i+6] = x;
    normales[i + 1] = y;  normales[i+4] = y; normales[i+7] = y;
    normales[i + 2] = z;  normales[i+5] = z; normales[i+8] = z;
    
    // printf("[%f, ", normales[i]);
    // printf("%f, ", normales[i + 1]);
    // printf("%f] \n", normales[i + 2]);
  }

}
