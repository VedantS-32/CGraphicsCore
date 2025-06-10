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
	vec3 uSkyboxTint;
	float uSkyboxIntensity;
};

layout(std140) uniform ModelCommons
{
	mat4 uView;
	mat4 uViewProjection;
	mat4 uSkyboxRotation;
};

layout(std140) uniform ModelProps
{
	int uEntityID;
	mat4 uModel;
};

layout(std430, binding = 1) buffer MaterialParameters
{
	float uIntensity;
	vec3 uSpecularColor;
	float uSpecularAlpha;
	float uTiling;
	vec3 uTint;
	float uReflectivity;
};

out VS_OUTWorldSettings
{
	vec3 CameraPosition;
	vec3 AmbientColor;
	vec3 LightPosition;
	vec3 SkyboxTint;
	float SkyboxIntensity;
} vWorldSettings;

out VS_OUTMaterialParams
{
	float Intensity;
	vec3 SpecularColor;
	float SpecularAlpha;
	float Tiling;
	vec3 Tint;
	float Reflectivity;
} vMaterialParams;

out VS_OUTModelProps
{
	int EntityID;
	vec2 TexCoord;
	vec3 WorldPos;
	vec3 Normal;
	mat3 Tbn;
	mat4 SkyboxRotation;
	vec3 ReflectedDir;
	vec3 SkyboxViewDir;
} vModelProps;

void main()
{
	mat4 vMVP = uViewProjection * uModel;

	vec3 tangent = normalize(vec3(uModel * vec4(aTangent, 0.0)));
	vec3 normal = normalize(vec3(uModel * vec4(aNormal, 0.0)));

	// Calculate world position
	vec3 worldPos = vec3(uModel * vec4(aPosition, 1.0));
	vec3 viewVector = normalize(uCameraPosition - worldPos);

	// Re-orthogonalize tangent with respect to normal
	tangent = normalize(tangent - dot(tangent, normal) * normal);

	// Retrieving perpendicular vector bitagent with the cross product of tangent and normal
	vec3 bitangent = cross(normal, tangent);

	mat3 tbn = mat3(tangent, bitangent, normal);
	
	vWorldSettings.CameraPosition = uCameraPosition;
	vWorldSettings.AmbientColor = uAmbientColor;
	vWorldSettings.LightPosition = uLightPosition;
	vWorldSettings.SkyboxIntensity = uSkyboxIntensity;
	vWorldSettings.SkyboxTint = uSkyboxTint;
	
	vMaterialParams.Intensity = uIntensity;
	vMaterialParams.SpecularColor = uSpecularColor;
	vMaterialParams.SpecularAlpha = uSpecularAlpha;
	vMaterialParams.Tiling = uTiling;
	vMaterialParams.Tint = uTint;
	vMaterialParams.Reflectivity = uReflectivity;

	vModelProps.EntityID = uEntityID;
	vModelProps.TexCoord = aTexCoord;
	vModelProps.WorldPos = worldPos;
	vModelProps.Normal = normalize(aNormal);
	vModelProps.Tbn = tbn;
	vModelProps.SkyboxRotation = uSkyboxRotation;
	vModelProps.ReflectedDir = reflect(-uLightPosition, vModelProps.Normal);
	vModelProps.SkyboxViewDir = viewVector;

	gl_Position = vMVP * vec4(aPosition, 1.0);
}

#type fragment
#version 450 core

uniform sampler2D uTextures[16];

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

uniform samplerCube uSkybox;

in VS_OUTWorldSettings
{
	vec3 CameraPosition;
	vec3 AmbientColor;
	vec3 LightPosition;
	vec3 SkyboxTint;
	float SkyboxIntensity;
} vWorldSettings;

in VS_OUTMaterialParams
{
	float Intensity;
	vec3 SpecularColor;
	float SpecularAlpha;
	float Tiling;
	vec3 Tint;
	float Reflectivity;
} vMaterialParams;

in VS_OUTModelProps
{
	flat int EntityID;
	vec2 TexCoord;
	vec3 WorldPos;
	vec3 Normal;
	mat3 Tbn;
	mat4 SkyboxRotation;
	vec3 ReflectedDir;
	vec3 SkyboxViewDir;
} vModelProps;

void main()
{
	vec3 diffuseMap = texture(uTextures[0], vModelProps.TexCoord * vMaterialParams.Tiling).rgb;
	vec3 normalMap = texture(uTextures[1], vModelProps.TexCoord * vMaterialParams.Tiling).rgb;

	vec3 normal = normalize(normalMap * 2.0 - 1.0);
	normal = normalize(vModelProps.Tbn * normal);
	vec3 reflected = normalize(vModelProps.ReflectedDir);
	vec3 lightPosition = normalize(vWorldSettings.LightPosition);

	float diffuse = max(dot(normal, lightPosition), 0.0);

	vec3 ambientColor = vWorldSettings.AmbientColor * vec3(diffuseMap);
	vec4 ambient = vec4(ambientColor, 1.0);

	vec3 halfAngle = normalize(lightPosition + vWorldSettings.CameraPosition);
	float blinn = max(dot(normal, halfAngle), 0.0);
	vec3 specular = vMaterialParams.SpecularColor * pow(blinn, vMaterialParams.SpecularAlpha);

	vec3 baseColor = diffuseMap * vMaterialParams.Tint;

	vec3 skyboxReflectedDir = reflect(vModelProps.SkyboxViewDir, normal);

	// Apply skybox rotation to reflection direction
	vec3 rotatedReflectedDir = vec3(vModelProps.SkyboxRotation * vec4(skyboxReflectedDir, 1.0));
	
	// Sample skybox with rotated direction
	vec3 envColor = texture(uSkybox, skyboxReflectedDir).rgb;
	envColor = envColor * vWorldSettings.SkyboxTint * vWorldSettings.SkyboxIntensity;
	
	vec4 litColor = vec4(baseColor * (vMaterialParams.Intensity * (diffuse + specular) + (envColor + ambientColor) * vMaterialParams.Reflectivity), 1.0);

	FragColor = litColor;
	//FragColor = vec4(envColor, 1.0);

	EntityID = vModelProps.EntityID;
}