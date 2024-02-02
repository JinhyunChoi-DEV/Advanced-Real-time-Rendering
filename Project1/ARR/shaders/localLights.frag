#version 430 core


layout (binding=0) uniform sampler2D gWorldPosition;
layout (binding=1) uniform sampler2D gNormalVector;
layout (binding=2) uniform sampler2D gDiffuse;
layout (binding=3) uniform sampler2D gSpecular;

uniform vec2 screenSize;

struct Light
{
	vec4 range;
	vec4 position;	
	vec4 color;
};

layout (std430) buffer LocalLights
{
	uint activeLightCount;
	Light light[];
} localLights;

in vec3 eye;
vec2 uv;
out vec4 FragColor;

void main()
{
	uv = gl_FragCoord.xy / screenSize;
	vec3 tPosition = texture(gWorldPosition, uv).rgb;
	vec3 tNormal = texture(gNormalVector, uv).rgb;
	vec3 tDiffuse = texture(gDiffuse, uv).rgb;
	vec4 tSpecular = texture(gSpecular, uv);

	if(!(length(tNormal) != 0))
		discard;	

	vec3 resultColor = vec3(0);

	for(int i = 0; i < localLights.activeLightCount; ++i)
	{
		Light light = localLights.light[i];

		vec3 lightVec = light.position.xyz - tPosition;
		vec3 eyeVec = eye - tPosition;
		vec3 N = normalize(tNormal);
		vec3 L = normalize(lightVec);
		vec3 V = normalize(eyeVec);
		vec3 H = normalize(L+V);
		float NL = max(dot(N,L),0.0);
		float NV = max(dot(N,V),0.0);
		float HN = max(dot(H,N),0.0);

		float distance = length(light.position.xyz - tPosition);
		if(distance < light.range.x)
		{
			vec3 diffsue = NL * light.color.xyz;
			vec3 specular = light.color.xyz * pow(HN, tSpecular.a);
			float att = 1/pow(distance, 2) - 1/pow(light.range.x, 2);
			resultColor += (diffsue + specular) * att;
		}
	}

	FragColor = vec4(resultColor,1);
}
