#include <windows.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glut.h"
#include "imgui/imconfig.h"

#include "imgui/imgui.h"
#include <ctype.h>          // toupper, isprint
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif
#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))         // Size of a static C-style array. Don't use on pointers!

#include <iostream>
#include <string>
#include <vector>

#include "InitShader.h"
#include "FpCamera.h"

#define BUFFER_OFFSET(i)    ((char*)NULL + (i))

//4 shaders, including tess. control and tess. evaluation
static const std::string vertex_shader("terrain_vs.glsl");
static const std::string tess_control_shader("terrain_tc.glsl");
static const std::string tess_eval_shader("terrain_te.glsl");
static const std::string fragment_shader("terrain_fs.glsl");
GLuint shader_program = -1;

//VAO and VBO for the tessellated patch
GLuint patch_vao = -1;
GLuint patch_vbo = -1;

//Number of terrain patches
const float patchX = 1.0f;
const float patchY = 1.0f;
const float patchScale = 20.0f;
const int numPatches = int(patchX*patchY);

int win_width = 1280;
int win_height = 720;

static GLfloat sampleX = 12.9898;
static GLfloat sampleY = 78.233;

static float lacunarity = 2.0f;
static int octaves = 8;
static float gain = 0.5;
int linear = 0;
int exponential = 1;
int logarithm = 2;

const glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
const glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

float diffuse_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

void draw_gui()
{
   ImGui_ImplGlut_NewFrame();

   const int n_sliders = 6;
   static float slider[n_sliders] = {60.0f, 60.0f, 60.0f, 60.0f, 60.0f, 60.0f};
   static float lacunarity = 2.0f;
   
   const std::string labels[n_sliders] = { "gl_TessLevelOuter[0]",
										   "gl_TessLevelOuter[1]",
										   "gl_TessLevelOuter[2]",
										   "gl_TessLevelOuter[3]",
										   "gl_TessLevelInner[0]", 
										   "gl_TessLevelInner[1]" };
   //for (int i = 0; i<n_sliders; i++)
   //{
   //   ImGui::SliderFloat(labels[i].c_str(), &slider[i], 1, 128);
   //}
   const char* items[] = { "Linear", "Exponential", "Logarithm" };
   static int item_current = 0;
   ImGui::Combo("Mode", &item_current, items, IM_ARRAYSIZE(items));
   ImGui::Spacing();
   ImGui::Spacing();

   int slider_loc = glGetUniformLocation(shader_program, "slider");
   glUniform1fv(slider_loc, n_sliders, slider);
   ImGui::NextColumn();

   ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 6.0f);
   int lacunarity_loc = glGetUniformLocation(shader_program, "lacunarity");
   glUniform1f(lacunarity_loc, lacunarity);
   
   ImGui::SliderInt("Octaves", &octaves, 1, 10);
   int octaves_loc = glGetUniformLocation(shader_program, "octaves");
   glUniform1i(octaves_loc, octaves);

   ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
   int gain_loc = glGetUniformLocation(shader_program, "gain");
   glUniform1f(gain_loc, gain);
  
   ImGui::Render();
}

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(shader_program);

   const int w = glutGet(GLUT_WINDOW_WIDTH);
   const int h = glutGet(GLUT_WINDOW_HEIGHT);
   const float aspect_ratio = float(w) / float(h);
   const float fov = 3.141592f / 2.0f;

   //Set up some uniform variables
   glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
   glm::mat4 V = GetViewMatrix();
   glm::mat4 M = glm::scale(glm::vec3(patchScale));   

   const int P_loc = 0;
   glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
   const int V_loc = 1;
   glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
   const int M_loc = 2;
   glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));

   glBindVertexArray(patch_vao);
   glPatchParameteri(GL_PATCH_VERTICES, 4); //number of input verts to the tess. control shader per patch.

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw terrain
   glDrawArrays(GL_PATCHES, 0, numPatches * 4); //Draw patches since we are using a tessellation shader.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   draw_gui();

   glutSwapBuffers();
}

void idle()
{
   glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   float time_sec = 0.001f*time_ms;

   const int time_loc = 3;
   glUniform1f(time_loc, time_sec);
}

void reload_shader()
{
   //Use the version of InitShader with 4 parameters. The shader names are in the order the stage are in the pipeline:
   //Vertex shader, Tess. control, Tess. evaluation, fragment shader
   GLuint new_shader = InitShader(vertex_shader.c_str(), tess_control_shader.c_str(), tess_eval_shader.c_str(), fragment_shader.c_str());

   if (new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if (shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;
   }
}

void reshape(int w, int h)
{
   win_width = w;
   win_height = h;
   glViewport(0, 0, w, h);
}

// glut keyboard callback function.
// This function gets called when an ASCII key is pressed
void keyboard(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyCallback(key);
   std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
   CameraKeyboard(key);
   switch (key)
   {
   case 'r':
   case 'R':
      reload_shader();
      break;
   }
}

void printGlInfo()
{
   std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
   std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
   std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void initOpenGl()
{
   glewInit();

   glEnable(GL_DEPTH_TEST);

   reload_shader();

   glGenVertexArrays(1, &patch_vao);
   glBindVertexArray(patch_vao);

   std::vector<glm::vec3> vertices;

   //DEMO: modify this to make a (patchX x patchY) grid of patches
   vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
   vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
   vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
   vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

   //create vertex buffers for vertex coords
   glGenBuffers(1, &patch_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, patch_vbo);
   glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

   int pos_loc = glGetAttribLocation(shader_program, "pos_attrib");
   if (pos_loc >= 0)
   {
      glEnableVertexAttribArray(pos_loc);
      glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
   }

   //Add diffuse color
   int diffuse_color_loc = glGetUniformLocation(shader_program, "diffuse_color");
   if (diffuse_color_loc != -1)
   {
	   glUniform4f(diffuse_color_loc, diffuse_color[0], diffuse_color[1], diffuse_color[2], diffuse_color[3]);
   }

   //calculate normal
   int normal_loc = -1;
   normal_loc = glGetAttribLocation(shader_program, "normal_attrib");

   InitCamera(glm::vec3(patchScale*patchX/2.0f, -patchScale*patchY/2.0f, 1.0f*patchScale), glm::vec3(3.14159265f/2.0f, 0.0f, 0.0f));
}

void keyboard_up(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyUpCallback(key);
}

void special_up(int key, int x, int y)
{
   ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y)
{
   ImGui_ImplGlut_PassiveMouseMotionCallback(x, y);
}

void special(int key, int x, int y)
{
   ImGui_ImplGlut_SpecialCallback(key);
   CameraSpecialKey(key);
}

void motion(int x, int y)
{
   ImGui_ImplGlut_MouseMotionCallback(x, y);
}

void mouse(int button, int state, int x, int y)
{
   ImGui_ImplGlut_MouseButtonCallback(button, state);
}

int main(int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition(5, 5);
   glutInitWindowSize(win_width, win_height);
   int win = glutCreateWindow("Lacunarity Terrain");

   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);
   glutSpecialFunc(special);
   glutKeyboardUpFunc(keyboard_up);
   glutSpecialUpFunc(special_up);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   glutPassiveMotionFunc(motion);
   glutReshapeFunc(reshape);

   initOpenGl();
   ImGui_ImplGlut_Init(); // initialize the imgui system

                          //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;
}
