#version 430 core

uniform sampler2D textureSampler;
uniform sampler2D normalSampler;

in vec2 interpTexCoord;
in vec3 lightDirTS;
in vec3 viewDirTS;


void main()
{
	vec3 L = normalize(lightDirTS);
	vec3 V = normalize(viewDirTS);
	vec3 N = texture2D(normalSampler, interpTexCoord).rgb;
	N = normalize((N * 2) - 1);
	vec3 R = reflect(-L, N);

	float diffuse = max(0, dot(N, L));
	
	float specular_pow = 20;
	float specular = pow(max(0, dot(R, V)), specular_pow);

	vec3 color = texture2D(textureSampler, interpTexCoord).rgb;

	vec3 lightColor = vec3(1.0);
	vec3 shadedColor = color * diffuse + lightColor * specular;
	
	float ambient = 0.1;

	gl_FragColor = vec4(mix(color, shadedColor, 1.0 - ambient), 1.0);
}
