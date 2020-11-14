#version 150

#extension GL_ARB_gpu_shader5 : enable

attribute vec3 a_pos;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform mat4 u_view;
uniform vec3 u_lightpos;

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

#define LINEAR_Z 1
//#define LOGARITHMIC_Z 1

const float C = 0.000001, near = 5.0, far = 1e9;

void main()
{
	mat4 modelView = u_view * u_model;
	vec4 pos = modelView * vec4(a_pos,1.0);

	mat3 normalMatrix = transpose(inverse(mat3(modelView)));

	v_normal = normalize(normalMatrix * a_normal);
	v_uv = a_uv;
	v_color = a_color;
	v_fragPos = vec3(u_model * vec4(a_pos,1.0));
	v_lightDir = normalize(u_lightpos - v_fragPos);

	vec4 p = u_projection * pos;

#if LINEAR_Z
	float fz = p.z * p.w;
	p.z = (fz - near) * 2.0 / (far - near) - 1.0;
#endif

#if LOGARITHMIC_Z
	p.z = (2 * log(C * p.w + 1) / log(C * far + 1) - 1) * p.w;
#endif

	gl_Position = p;
}