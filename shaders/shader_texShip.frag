#version 430 core

uniform sampler2D textureSampler;
uniform sampler2D normalSampler;

in vec2 interpTexCoord;
in vec3 lightDirTS1;
in vec3 lightDirTS2;
in vec3 viewDirTS;


void main()
{
	// pierwsze Ÿród³o œwiat³a
	vec3 L = normalize(lightDirTS1);
	vec3 V = normalize(viewDirTS);
	vec3 N = texture2D(normalSampler, interpTexCoord).rgb;
	N = normalize((N * 2) - 1);
	vec3 R = reflect(-L, N);

    float diffuse = max(0, dot(N, L));
	
	float specular_pow = 20;
	float specular = pow(max(0, dot(R, V)), specular_pow);

    float dis = length(L);
	vec3 lightColor =  vec3(1.0, 0.95, 0.85);
    vec3 attenuation = lightColor / (1.0f + 0.09 * dis + 0.032 * (dis * dis)); 

	vec3 color = texture2D(textureSampler, interpTexCoord).rgb;
	float ambient = 0.1;

	vec3 shadedColor = color * diffuse * attenuation + lightColor * specular * attenuation;

	vec4 result = vec4(mix(color, shadedColor, 1.0 - ambient), 1.0);

	// drugie Ÿród³o œwiat³a
	L = normalize(lightDirTS2);
	R = reflect(-L, N);

    diffuse = max(0, dot(N, L));
	
	specular_pow = 20;
	specular = pow(max(0, dot(R, V)), specular_pow);

    dis = length(L);
	lightColor =  vec3(0.5, 0.1, 0.0);
    attenuation = lightColor / (1.0f + 0.09 * dis + 0.032 * (dis * dis)); 

	shadedColor = color * diffuse * attenuation + lightColor * specular * attenuation;

	result += vec4(mix(color, shadedColor, 1.0 - ambient), 1.0);

	gl_FragColor = vec4(result.xyz, 1.0);
}
