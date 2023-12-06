#version 450 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 textureCoordinate;

layout (set = 0, binding = 0) uniform sampler2D imageSampler;
layout (set = 0, binding = 1) uniform sampler2D depthImageSampler;

void main()
{
	color = texture(imageSampler, textureCoordinate);
	gl_FragDepth = texture(depthImageSampler, textureCoordinate).r;
    
    //if(color.w == 1.0)
    //    color.w = 0.5;
    //if(color.w < 0.9)
    //    gl_FragDepth = 0.9;
    //else
    //    gl_FragDepth = 0.99;
}
