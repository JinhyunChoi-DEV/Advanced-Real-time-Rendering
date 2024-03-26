#version 430 core


layout (binding=0) uniform sampler2D gWorldPosition;
layout (binding=1) uniform sampler2D gNormalVector;
layout (binding=2) uniform sampler2D gDiffuse;
layout (binding=3) uniform sampler2D gSpecular;

uniform vec2 screenSize;
uniform bool isPBR;

struct Light
{
	vec4 position;		// position.w is range
	vec4 color;
};

layout (std430) buffer LocalLights
{
	uint activeLightCount;
	Light light[];
} localLights;

const float pi = 3.14159;

in vec3 eye;
vec2 uv;
out vec4 FragColor;

float D(vec3 N, vec3 H, float a);
vec3 F(vec3 L, vec3 H, vec3 Ks);
float G(vec3 L, vec3 H);
vec2 getUV(vec3 L);

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

		float range = light.position.w;
		vec3 position = light.position.xyz;
		vec3 color = light.color.rgb;

		vec3 lightVec = position - tPosition;
		vec3 eyeVec = eye - tPosition;
		vec3 N = normalize(tNormal);
		vec3 L = normalize(lightVec);
		vec3 V = normalize(eyeVec);
		vec3 H = normalize(L+V);
		float NL = max(dot(N,L),0.0);
		float NV = max(dot(N,V),0.0);
		float HN = max(dot(H,N),0.0);

		float distance = length(position - tPosition);
		if(distance < range)
		{
			if(isPBR)
			{
				float att = 1/pow(distance, 2) - 1/pow(range, 2);
				vec3 Kd = color;
				vec3 Ks = tSpecular.rgb;
				float shininess = tSpecular.a;
				float d = D(N, H, shininess);
	    		vec3 f = F(L, H, Ks);
	    		float g = G(L, H);

	    		resultColor += (Kd/pi + (d*f*g)/4.0f) * att;

			}
			else
			{
				vec3 diffsue = NL * color;
				vec3 specular = color * pow(HN, tSpecular.a);
				float att = 1/pow(distance, 2) - 1/pow(range, 2);
				resultColor += (diffsue + specular) * att;
			}
		}
	}
	FragColor = vec4(resultColor,1);
}

float D(vec3 N, vec3 H, float a)
{
	return (a+2.0)/(2.0*pi) * pow(max(dot(N,H), 0.0), a);
}

vec3 F(vec3 L, vec3 H, vec3 Ks)
{
	return Ks + (1.0f - Ks) * pow((1.0f - max(dot(L,H), 0.0)),5);
}

float G(vec3 L, vec3 H)
{
	return 1.0  / pow(dot(L, H), 2);
}

vec2 getUV(vec3 L)
{
	float u = (0.5f - atan(L.y, L.x)/(2.0*pi));
	float v = acos(L.z) / pi;

	return vec2(u, v);
}