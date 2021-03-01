#version 430 core

uniform sampler2D textureSampler;

in vec3 interpNormal;
in vec3 fragPos;
in vec2 vertexTextureCoord;

void main()
{
	vec4 textureColor = texture2D(textureSampler, -vertexTextureCoord);		// minus przed vertexTextureCoord obraca teksturê o 180*
	gl_FragColor.rgb = textureColor.xyz;
	gl_FragColor.a = 1.0;
}
