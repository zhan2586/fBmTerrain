#include "FpCamera.h"
#include <glm/gtx/transform.hpp>
#include <GL/freeglut.h>

static const float cameraMoveSpeed = 0.5f;
static const float cameraTurnSpeed = 0.05f;

static glm::vec3 CamPos(0.0f);
static glm::vec3 CamRot(0.0f);
static glm::mat4 C(1.0f);

glm::mat4 GetViewMatrix()
{
   return glm::inverse(C);
}

glm::vec3 GetCameraPosition()
{
   return CamPos;
}

void UpdateC()
{
   C = glm::translate(CamPos) *
      glm::rotate(CamRot.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
      glm::rotate(CamRot.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(CamRot.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void InitCamera(const glm::vec3& pos, const glm::vec3& rot)
{
   CamPos = pos;
   CamRot = rot;
   UpdateC();
}

void CameraSpecialKey(int key)
{
   if (key == GLUT_KEY_UP) { CamPos += glm::vec3(C * glm::vec4(0.0f, 0.0f, -cameraMoveSpeed, 0.0f)); }
   if (key == GLUT_KEY_DOWN) { CamPos += glm::vec3(C * glm::vec4(0.0f, 0.0f, +cameraMoveSpeed, 0.0f)); }

   if (key == GLUT_KEY_RIGHT) {CamRot.z -= cameraTurnSpeed;}
   if (key == GLUT_KEY_LEFT) {CamRot.z += cameraTurnSpeed;}

   UpdateC();
}

void CameraKeyboard(unsigned char key)
{
   if (key == 'a') { CamPos += glm::vec3(C * glm::vec4(0.0f, +cameraMoveSpeed, 0.0f, 0.0f)); }
   if (key == 'z') { CamPos += glm::vec3(C * glm::vec4(0.0f, -cameraMoveSpeed, 0.0f, 0.0f)); }

   UpdateC();
}
 