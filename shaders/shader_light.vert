#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 cameraPos;

out vec3 interpNormal;
out vec3 lightDirTS1;
out vec3 lightDirTS2;
out vec3 viewDirTS;

void main()
{
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
	vec3 vertPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;

	vec3 fragPos = (modelMatrix*vec4(vertexPosition,1)).xyz;
	vec3 lightDir1 = normalize(lightPos1 - fragPos);
	vec3 lightDir2 = normalize(lightPos2 - fragPos);

	interpNormal = (modelMatrix*vec4(vertexNormal,0)).xyz;

	vec3 normal, tangent, bitangent;
	normal = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
	tangent = (modelMatrix * vec4(vertexTangent, 0.0)).xyz;
	bitangent = (modelMatrix * vec4(vertexBitangent, 0.0)).xyz;

	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 viewDir = normalize(cameraPos - vertPos);

	lightDirTS1 = TBN * lightDir1;
	lightDirTS2 = TBN * lightDir2;
	viewDirTS = TBN * viewDir;
}

