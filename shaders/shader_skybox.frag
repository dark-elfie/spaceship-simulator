#version 430 core

in vec2 vertexTextureCoord;

uniform sampler2D textureSampler;

void main()
{    
    vec4 textureColor = texture2D(textureSampler, -vertexTextureCoord);		// minus przed vertexTextureCoord obraca teksturê o 180*
	gl_FragColor.rgb = textureColor.xyz;
	gl_FragColor.a = 1.0;
}
