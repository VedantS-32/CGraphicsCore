#type vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform WorldSettings
{
	vec3 uCameraPosition;
	vec3 uAmbientColor;
	vec3 uLightPosition;
};

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

out vec3 vCameraPosition;
out vec3 vAmbientColor;
out vec3 vLightPosition;

out vec4 vColor;
out float vIntensity;
out vec3 vNormal;

void main()
{
	mat4 vMVP = uViewProjection * uModel;

	vCameraPosition = uCameraPosition;
	vAmbientColor = uAmbientColor;
	vLightPosition = uLightPosition;

	vColor = uColor;
	vIntensity = uIntensity;
	vNormal = aNormal;

	gl_Position = vMVP * vec4(aPosition, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in vec3 vCameraPosition;
in vec3 vAmbientColor;
in vec3 vLightPosition;

in vec4 vColor;
in float vIntensity;
in vec3 vNormal;

void main()
{
	vec3 normal = normalize(vNormal);
	float diffuse = max(dot(vNormal, vLightPosition), 0.0);
	FragColor = (vColor * (vIntensity * diffuse)) + vec4(vAmbientColor, 1.0);
}