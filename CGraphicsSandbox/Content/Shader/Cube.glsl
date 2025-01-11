#type vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform ModelCommons
{
	mat4 uView;
	mat4 uViewProjection;
};

layout(std140) uniform ModelProps
{
	mat4 uModel;
};

layout(std430, binding = 1) buffer MaterialParameters
{
	float uIntensity;
	vec4 uColor;
};

out vec4 vColor;
out float vIntensity;
out vec3 vNormal;

void main()
{
	mat4 vMVP = uViewProjection * uModel;
	vColor = uColor;
	vIntensity = uIntensity;
	vNormal = aNormal;

	gl_Position = vMVP * vec4(aPosition, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in vec4 vColor;
in float vIntensity;
in vec3 vNormal;
vec3 LightPosition = normalize(vec3(1.0, -1.0, 1.0));

void main()
{
	float diffuse = max(dot(vNormal, LightPosition), 0.0);
	FragColor = vIntensity * diffuse * vColor + vec4(0.1, 0.1, 0.15, 1.0);
	//FragColor = vColor;
}