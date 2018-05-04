#version 430

uniform sampler2D texture;
uniform vec3 lightPos;  
uniform vec3 lightColor;
uniform vec3 objectColor;

//in vec4 diffuse;
in vec3 world_normal;
in vec2 tex_coord; 
in vec3 FragPos; 

out vec4 fragcolor; 

vec3 norm = normalize(world_normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);

void main(void)
{   
	//float cosTheta = clamp(dot(n, l), 0, 1);
	vec3 diffuse = diff * lightColor;
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
	vec3 result = (ambient + diffuse) * objectColor;
	fragcolor = vec4(result, 1.0);
	fragcolor = vec4(1.0,1.0,0.5,1.0);	
}

