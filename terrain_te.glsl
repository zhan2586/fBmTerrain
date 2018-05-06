#version 430 core

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 3) uniform float time;

layout (quads, equal_spacing, ccw) in;	//discrete LOD
//Try some of these other options
//layout (quads, fractional_odd_spacing, ccw) in;	//continuous LOD
//layout (quads, fractional_even_spacing, ccw) in;	//continuous LOD

uniform float lacunarity;
uniform int octaves;
uniform float gain;
uniform int item_current;

out float dist;

// Author @patriciogv - 2015
// http://patriciogonzalezvivo.com

#ifdef GL_ES
precision mediump float;
#endif

float random (in vec2 st) {
    return fract(sin(dot(st.xy, vec2(40.5,20.8)))*43758.5453123);
	//return fract(sin(dot(st.x,sampleX),sampleY)))*43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

//#define OCTAVES 6

float fbm (in vec2 st) {
    // Initial values
	vec2 coord = st;
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 0.0;	
  
    // Loop of octaves
    for (int i = 0; i < octaves; i++) 
	{
        value += amplitude * noise(coord);

		if(item_current == 0)
		{			
			coord = coord * lacunarity;
		}
        if(item_current == 1)
		{			
			coord = coord * exp2(lacunarity);
		}
		if(item_current == 2)
		{			
			coord = coord * log2(lacunarity);
		}
		amplitude *= gain;  //amplitude *= 0.5;	
	}
    return value;
}

void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	const vec4 p3 = gl_in[3].gl_Position;

	vec4 p = (1.0-u)*(1.0-v)*p0 + u*(1.0-v)*p1 + u*v*p2 + (1.0-u)*v*p3;

	//DEMO: apply terrain height offset in z-direction
	p.z += fbm(p.xy);
	vec4 p_eye = V*M*p;
	dist = p_eye.z;

	gl_Position = P * p_eye;
}
