#type vertex
#version 330

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main()
{
	vColor = aColor;
	gl_Position = vec4(aPosition, 0.0, 1.0);
}

#type fragment
#version 330

in vec3 vColor;
out vec4 FragColor;

void main()
{
	FragColor = vec4(vColor, 1.0);
}