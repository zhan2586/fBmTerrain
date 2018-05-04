#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 10) uniform float slider[6];

layout (vertices = 4) out;  //number of output verts of the tess. control shader

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	/*DEMO: Idea, let the tessellation levels depend on distance from the eye.

						3-------2
			eye.		|		|
						|		|
						0-------1
	
			Make gl_TessLevelOuter[0] depend on distance from eye to edge 3,0.
			Make gl_TessLevelOuter[1] depend on distance from eye to edge 0,1.
			Etc...

			TODO: write 2 functions: 
				float dist_point_to_line_segment(vec3 p, vec3 l0, vec3 l1);
				float outer_tess_level(float d);

				Let inner tesselation levels be averages of outer tessellation levels.
	*/

	gl_TessLevelOuter[0] = slider[0];
	gl_TessLevelOuter[1] = slider[1];
	gl_TessLevelOuter[2] = slider[2];
	gl_TessLevelOuter[3] = slider[3];

	gl_TessLevelInner[0] = slider[4];
	gl_TessLevelInner[1] = slider[5];
}
