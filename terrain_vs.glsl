#version 430 

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 diffuse_color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

in vec3 pos_attrib;
in vec3 normal_attrib; 

//out vec4 diffuse;
out vec3 world_normal;
out vec3 FragPos;

void main(void)
{
	world_normal = normal_attrib;
	//diffuse = diffuse_color;// * vec4(max(dot(l, world_normal), 0));	
	FragPos = vec3(M * vec4(pos_attrib, 1.0));

	gl_Position = vec4(pos_attrib, 1.0);	
}