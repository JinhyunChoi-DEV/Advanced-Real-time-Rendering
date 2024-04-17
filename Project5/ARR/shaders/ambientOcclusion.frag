#version 430 core

uniform sampler2D gWorldPosition;
uniform sampler2D gNormalVector;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
uniform sampler2D gDepth;

uniform float R;
uniform int n;
uniform float k;
uniform float s;
uniform vec2 screenSize;

const float pi = 3.141592653589793;

float HeavisideFunction(vec3 omegaI);

out vec3 AOColor;

void main()
{
	vec2 uv = gl_FragCoord.xy / screenSize;
	ivec2 coeff_int = ivec2(gl_FragCoord.xy);
	vec2 coeff_float = gl_FragCoord.xy / screenSize;

	vec3 P = texture2D(gWorldPosition, uv).rgb;
	vec3 N = texture2D(gNormalVector, uv).rgb;
	float d = texture2D(gDepth, uv).r;
	float c = 0.1 * R;

	float phi = (30*coeff_int.x ^ coeff_int.y) + 10 * coeff_int.x * coeff_int.y;
	float result = 0.0;
	for(int i = 0; i < n; ++i)
	{
		float a = (i + 0.5) / n;
		float h = a*R/d;
		float theta = 2.0*pi*a*(7.0*n/9.0) + phi;

		vec2 newUV = coeff_float + h*vec2(cos(theta), sin(theta));
		vec3 Pi = texture2D(gWorldPosition, newUV).rgb;
		vec3 omegaI = Pi - P;
		float di = Pi.z;

		result += (max(0.0, dot(N,omegaI) - 0.001*di) * HeavisideFunction(omegaI)) / max(c*c, dot(omegaI, omegaI));
	}

	result *= (2.0*pi*c)/n;
	result = max(0.0, pow(1.0-s*result, k));
	AOColor = vec3(result);
}


float HeavisideFunction(vec3 omegaI)
{
	float result = R - length(omegaI);

	if(result < 0)
		return 0.0f;

	return 1.0f;
}
