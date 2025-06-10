#type vertex
#version 450 core

layout(location = 0) in vec3 aPosition;

out vec3 vTexCoord;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{    
    vec4 pos = uProjection * mat4(mat3(uView)) * vec4(aPosition, 1.0f);
    vTexCoord = aPosition;
    
    gl_Position = pos.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

in vec3 vTexCoord;

layout(std140) uniform WorldSettings
{
	vec3 uCameraPosition;
	vec3 uAmbientColor;
	vec3 uLightPosition;
	vec3 uSkyboxTint;
	float uSkyboxIntensity;
};

uniform samplerCube uSkybox;

void main()
{
    FragColor = texture(uSkybox, vTexCoord) * vec4(uSkyboxTint, 1.0f) * uSkyboxIntensity;
    EntityID = -1;
}