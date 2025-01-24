#type vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

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
	float uTiling;
	vec3 uTint;
};

out VS_OUTWorldSettings
{
	vec3 CameraPosition;
	vec3 AmbientColor;
	vec3 LightPosition;
} vWorldSettings;

out VS_OUTMaterialParams
{
	vec3 Color;
	float Intensity;
	vec3 SpecularColor;
	float SpecularAlpha;
	float Tiling;
	vec3 Tint;
} vMaterialParams;

out VS_OUTModelProps
{
	vec2 TexCoord;
	vec3 Normal;
	mat3 Tbn;
	vec3 ReflectedDir;
} vModelProps;

void main()
{
	mat4 vMVP = uViewProjection * uModel;

	vec3 tangent = normalize(vec3(uModel * vec4(aTangent, 0.0)));
	vec3 normal = normalize(vec3(uModel * vec4(aNormal, 0.0)));

	//// Re-orthogonalize tangent with respect to normal
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	//// Retrieving perpendicular vector bitagent with the cross product of tangent and normal
	vec3 bitangent = cross(normal, tangent);

	mat3 tbn = mat3(tangent, bitangent, normal);
	
	vWorldSettings.CameraPosition = uCameraPosition;
	vWorldSettings.AmbientColor = uAmbientColor;
	vWorldSettings.LightPosition = uLightPosition;
	
	vMaterialParams.Color = uColor;
	vMaterialParams.Intensity = uIntensity;
	vMaterialParams.SpecularColor = uSpecularColor;
	vMaterialParams.SpecularAlpha = uSpecularAlpha;
	vMaterialParams.Tiling = uTiling;
	vMaterialParams.Tint = uTint;

	vModelProps.TexCoord = aTexCoord;
	vModelProps.Normal = aNormal;
	vModelProps.Tbn = tbn;
	vModelProps.ReflectedDir = reflect(-uLightPosition, aNormal);

	gl_Position = vMVP * vec4(aPosition, 1.0);
}

#type fragment
#version 450 core

uniform sampler2D uTextures[16];

out vec4 FragColor;

in VS_OUTWorldSettings
{
	vec3 CameraPosition;
	vec3 AmbientColor;
	vec3 LightPosition;
} vWorldSettings;

in VS_OUTMaterialParams
{
	vec3 Color;
	float Intensity;
	vec3 SpecularColor;
	float SpecularAlpha;
	float Tiling;
	vec3 Tint;
} vMaterialParams;

in VS_OUTModelProps
{
	vec2 TexCoord;
	vec3 Normal;
	mat3 Tbn;
	vec3 ReflectedDir;
} vModelProps;

void main()
{
	vec3 normal = texture(uTextures[1], vModelProps.TexCoord * vMaterialParams.Tiling).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normal * 2.0 - 1.0;
	normal = normalize(vModelProps.Tbn * normal);
	vec3 reflected = normalize(vModelProps.ReflectedDir);
	vec3 lightPosition = normalize(vWorldSettings.LightPosition);

	float diffuse = max(dot(normal, lightPosition), 0.0);
	vec4 ambient = vec4(vWorldSettings.AmbientColor * vec3(texture(uTextures[0], vModelProps.TexCoord)), 1.0);
	float phong = max(dot(reflected, vWorldSettings.CameraPosition), 0.0);
	vec3 halfAngle = (lightPosition + vWorldSettings.CameraPosition) / length(lightPosition + vWorldSettings.CameraPosition);
	float blinn = max(dot(normal, halfAngle), 0.0);

	//FragColor = vec4(normal, 1.0);
	//FragColor = vec4(vColor, 1.0) * (vIntensity * (diffuse + ((vec4(vSpecularColor, 1.0) * pow(blinn, vSpecularAlpha)))) + vec4(vAmbientColor, 1.0));
	FragColor = (texture(uTextures[0], vModelProps.TexCoord * vMaterialParams.Tiling) * vec4(vMaterialParams.Tint, 1.0)) *
	(vMaterialParams.Intensity * (diffuse + ((vec4(vMaterialParams.SpecularColor, 1.0) *
	pow(blinn, vMaterialParams.SpecularAlpha)))) + ambient);
}