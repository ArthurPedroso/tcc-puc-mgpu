#version 450 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec3 f_normal;

layout (set = 0, binding = 0) uniform Material
{
	vec4 ambient, diffuse;
	float opacity;
} material;

void main()
{
	float intensity = clamp(dot(normalize(vec3(-1.0, 0.0, 1.0)), f_normal), 0, 1);
	color = vec4(max(material.ambient, material.diffuse * intensity).xyz, material.opacity);
	//gl_FragDepth = 0.99;
}