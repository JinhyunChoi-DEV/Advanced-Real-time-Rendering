#version 430 core

struct MainLight
{
	vec3 position;
	vec3 color;
};

uniform uint mode;			// 0: total, 1: world, 2: normal, 3: diffuse, 4: specular, 5: shadow map

uniform MainLight mainLight;

uniform sampler2D gWorldPosition;
uniform sampler2D gNormalVector;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
uniform sampler2D shadowMap;

uniform mat4 shadowMatrix;
uniform vec2 screenSize;
uniform float alpha;
uniform float near; 
uniform float far;

in vec3 eye;
out vec4 FragColor;

vec2 uv;
vec3 GetColor();
vec3 cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3);
float CaluateShadow(vec4 worldPos);

void main()
{
	uv = gl_FragCoord.xy / screenSize;

	if(mode == 5)
	{
		vec4 tPositionV4 = texture2D(gWorldPosition, uv);
		vec4 tShadow = texture(shadowMap, uv);
		FragColor.xyz  = tShadow.rgb;	
	}
	else
	{
		vec3 tNormal = texture(gNormalVector, uv).rgb;

		if(!(length(tNormal) != 0))
			discard;

		vec3 resultColor = GetColor();
		FragColor.xyz = resultColor;
	}
}

vec3 GetColor()
{	
	vec3 tPosition = texture2D(gWorldPosition, uv).rgb;
	vec4 tPositionV4 = texture2D(gWorldPosition, uv);
	vec3 tNormal = texture2D(gNormalVector, uv).rgb;
	vec3 tDiffuse = texture2D(gDiffuse, uv).rgb;
	vec4 tSpecular = texture2D(gSpecular, uv);

	float distance = length(mainLight.position - tPosition);
	vec3 Ia = mainLight.color * 0.2;
	vec3 I = mainLight.color;
	float shadow = CaluateShadow(tPositionV4);

	if(mode == 0)
	{
		vec3 lightVec = mainLight.position - tPosition;
		vec3 eyeVec = eye - tPosition;
		vec3 N = normalize(tNormal);
		vec3 L = normalize(lightVec);
	    vec3 V = normalize(eyeVec);
	    vec3 H = normalize(L+V);
	    float NL = max(dot(N,L),0.0);
	    float NV = max(dot(N,V),0.0); 
	    float HN = max(dot(H,N),0.0);
	    float shininess = tSpecular.a;

	    vec3 diffsue = I * NL * tDiffuse;
		vec3 specular = I * tSpecular.rgb * pow(HN, shininess);

	    vec3 result = vec3(0.0f);
	    result = (Ia * tDiffuse + diffsue + specular) * (1-shadow);

		return result;
	}
	else if(mode == 1)
	{
		return abs(tPosition);
	}
	else if(mode == 2)
	{
		return abs(tNormal);
	}
	else if(mode ==3)
	{
		return tDiffuse;
	}
	else if(mode == 4)
	{
		return tSpecular.rgb;
	}	
}

vec3 cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3)
{
    float a = sqrt(m11);
    float b = m12/a;
    float c = m13/a;
    float d = sqrt(m22-(b*b));
    float e = (m23-(b*c))/d;
    float f = sqrt(m33-(c*c)-(e*e));
    float c1_hat = z1/a;
    float c2_hat =(z2-b*c1_hat)/d;
    float c3_hat =(z3-(c*c1_hat)-(e*c2_hat))/ f;

    float c3 = c3_hat /f;
    float c2 =(c2_hat-(e*c3))/d;
    float c1 =(c1_hat-(b*c2)-(c*c3))/a;
    return vec3(c1,c2,c3);
}

float CaluateShadow(vec4 worldPos)
{
	vec4 shadowCoord = shadowMatrix * worldPos;
	vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;
	shadowIndex = shadowIndex*0.5f + 0.5f;

	float shadow = 0;
	float zf = log2(shadowCoord.w - near + 1.0f) / log2(far - near + 1.0f);
	vec4 b = texture2D(shadowMap, shadowIndex);
	vec4 b_prime = (1-alpha)*b + alpha*vec4(0.5);
	vec3 c = cholesky(1, b_prime.x, b_prime.y, b_prime.y, b_prime.z, b_prime.w, 1, zf, zf*zf);
	float z2,z3;
	float D = c.y * c.y - 4.0*c.z*c.x;
	if(D<0)
	{
		z2 = (-c.y)/(2*c.z);
		z3 = z2;
	}
	else
	{
		z2 = (-c.y-sqrt(c.y*c.y - 4*c.z*c.x))/(2*c.z);
		z3 = (-c.y+sqrt(c.y*c.y - 4*c.z*c.x))/(2*c.z);
	}
	float tempV = z2;
	if(z2 > z3)
	{
		z2 = z3;
		z3 = tempV;
	}
	if(zf <= z2)
		return 0;
	else if(zf <= z3)
		return ((zf*z3)-(b_prime.x*(zf+z3))+b_prime.y)/((z3-z2) * (zf-z2));
	else
		return 1.0 - (z2*z3 - b_prime.x*(z2+z3)+b_prime.y)/((zf-z2)*(zf-z3));
}