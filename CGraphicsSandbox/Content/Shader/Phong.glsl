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
	vec3 uColor;
	vec3 uSpecularColor;
	float uSpecularAlpha;
};

out vec3 vCameraPosition;
out vec3 vAmbientColor;
out vec3 vLightPosition;

out vec3 vColor;
out float vIntensity;
out vec3 vSpecularColor;
out float vSpecularAlpha;

out vec3 vNormal;
out vec3 vReflectedDir;

void main()
{
	mat4 vMVP = uViewProjection * uModel;
	
	vCameraPosition = uCameraPosition;
	vAmbientColor = uAmbientColor;
	vLightPosition = normalize(uLightPosition);
	
	vColor = uColor;
	vIntensity = uIntensity;
	vSpecularColor = uSpecularColor;
	vSpecularAlpha = uSpecularAlpha;

	vNormal = aNormal;
	vReflectedDir = reflect(-uLightPosition, vNormal);

	gl_Position = vMVP * vec4(aPosition, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

in vec3 vCameraPosition;
in vec3 vAmbientColor;
in vec3 vLightPosition;

in vec3 vColor;
in float vIntensity;
in vec3 vSpecularColor;
in float vSpecularAlpha;

in vec3 vNormal;
in vec3 vReflectedDir;

void main()
{
	vec3 normal = normalize(vNormal);
	vec3 reflected = normalize(vReflectedDir);

	float diffuse = max(dot(normal, vLightPosition), 0.0);
	float phong = max(dot(reflected, vCameraPosition), 0.0);
	vec3 halfAngle = (vLightPosition + vCameraPosition) / length(vLightPosition + vCameraPosition);
	float blinn = max(dot(normal, halfAngle), 0.0);

	FragColor = vec4(vColor, 1.0) * (vIntensity * (diffuse + ((vec4(vSpecularColor, 1.0) * pow(blinn, vSpecularAlpha)))) + vec4(vAmbientColor, 1.0));
}