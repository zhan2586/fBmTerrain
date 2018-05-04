#pragma once

#include <glm/glm.hpp>

//Set initial camera position and orientation
void InitCamera(const glm::vec3& pos, const glm::vec3& rot);

//Pass the key from glutSpecialFunc
void CameraSpecialKey(int key);

//Pass the key from glutKeyboardFunc
void CameraKeyboard(unsigned char key);

//Get the camera view matrix
glm::mat4 GetViewMatrix(); 

//Get the camera position
glm::vec3 GetCameraPosition();
