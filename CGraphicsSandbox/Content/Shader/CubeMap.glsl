#type vertex
#version 450 core

layout(location = 0) in vec2 aPosition;

out vec3 vDirection;

uniform mat4 uInverseVP;

void main()
{
    vec4 front = uInverseVP * vec4(aPosition, -0.9999, 1.0);
    vec4 back = uInverseVP * vec4(aPosition, 0.9999, 1.0);
    
    vDirection = back.xyz / back.w - front.xyz / front.w;
    
    gl_Position = vec4(aPosition, 0.9999, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

in vec3 vDirection;

uniform samplerCube uSkybox;

void main()
{
    FragColor = texture(uSkybox, vDirection);
    EntityID = -1;
}