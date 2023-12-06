#version 450 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec3 f_normal;
layout (location = 1) in vec2 f_uv;

layout (set = 0, binding = 0) uniform Material
{
	vec4 ambient, diffuse;
	float opacity;
} material;

layout (set = 0, binding = 1) uniform sampler2D m_sampler;

void main()
{
	//Calculo de iluminação simples
	float intensity = clamp(dot(normalize(vec3(-1.0, 0.0, 1.0)), f_normal), 0, 1);

	vec4 matProps = max(material.ambient, material.diffuse * intensity);
	matProps.w *= material.opacity;
	vec4 textureColor = texture(m_sampler, f_uv);
	color = matProps * textureColor;

	//gl_FragDepth = 0.9;
}