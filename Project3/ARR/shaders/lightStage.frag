#version 430 core

struct MainLight
{
	vec3 position;
	vec3 color;
};

uniform uint mode;			// 0: PBR, 1: Phong, 2: world, 3: normal, 4: diffuse, 5: specular, 6: shadow map

uniform MainLight mainLight;

uniform sampler2D gWorldPosition;
uniform sampler2D gNormalVector;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
uniform sampler2D shadowMap;
uniform sampler2D backgroundTexture; 
uniform sampler2D irradMap;

uniform mat4 shadowMatrix;
uniform vec2 screenSize;
uniform float alpha;
uniform float near; 
uniform float far;
uniform float exposureControl;

uniform HammersleyBlock
{
	float N;
	float hammersley[2*100];
} HB;

in vec3 eye;
out vec4 FragColor;

const float pi = 3.14159;

vec2 uv;
vec3 GetColor();
vec3 cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3);
float CaluateShadow(vec4 worldPos);

float D(vec3 N, vec3 H, float a);
vec3 F(vec3 L, vec3 H, vec3 Ks);
float G1(vec3 v, vec3 N, float shininess);
float G(vec3 L, vec3 V, vec3 N, float shininess);
vec2 getUV(vec3 L);

void main()
{
	uv = gl_FragCoord.xy / screenSize;

	if(mode == 6)
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
	vec3 Kd = tDiffuse.rgb;
	vec3 Ks = tSpecular.rgb;

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
	    vec3 specularColor = vec3(0.0f);
	    vec3 diffuseColor = texture2D(irradMap, getUV(N)).rgb* (Kd / pi);
	    for(int i = 0; i < HB.N; i++)
	    {
	    	float o1 = HB.hammersley[i*2];
	    	float o2 = HB.hammersley[i*2+1];

	    	float theta = acos(pow(o2, 1.0f/(shininess+1.0f)));
	    	vec2 curUV = vec2(o1, theta/pi);
	    	vec3 L_ = vec3(cos(2.0f*pi*(0.5f-curUV.x))*sin(pi*curUV.y), sin(2.0f*pi*(0.5f-curUV.x))*sin(pi*curUV.y), cos(pi*curUV.y));
	    	vec3 R = 2.0f * dot(N,V)*N - V;
	    	vec3 A = normalize(vec3(-R.y, R.x, 0));
	    	vec3 B = normalize(cross(R,A));
	    	vec3 omegaK = normalize(L_.x * A + L_.y * B + L_.z * R);

	    	vec3 H_ = normalize(omegaK+V);
	    	float level = 0.5f*log2((screenSize.x * screenSize.y) / HB.N) - 0.5f*log2(D(N, H_, shininess));
	    	vec2 backUV = getUV(omegaK);
	    	vec4 Li = textureLod(backgroundTexture, backUV, level);
	    	specularColor += (G(omegaK, V, N, shininess) * F(omegaK, H, Ks))/(4.0f*dot(omegaK,N)*NV) * Li.xyz * cos(theta);
	    }
	    specularColor /= HB.N;
	    specularColor = pow((exposureControl*specularColor)/((exposureControl*specularColor)+vec3(1,1,1)), vec3(1.0/2.2f));

	    vec3 result = vec3(0.0f);
	    result = (diffuseColor + specularColor) * (1-shadow);
	    return result;
	}
	else if(mode == 1)
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
	else if(mode == 2)
	{
		return abs(tPosition);
	}
	else if(mode == 3)
	{
		return abs(tNormal);
	}
	else if(mode == 4)
	{
		return tDiffuse;
	}
	else if(mode == 5)
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

float D(vec3 N, vec3 H, float a)
{
	return (a+2.0)/(2.0*pi) * pow(max(dot(N,H), 0.0), a);
}

vec3 F(vec3 L, vec3 H, vec3 Ks)
{
	return Ks + (vec3(1.0f) - Ks) * pow((1.0f - max(dot(L,H), 0.0)),5);
}

float G1(vec3 v, vec3 N, float shininess)
{
	float VN = max(dot(v, N), 0.0);
	if(VN > 1.0f)
		return 1.0f;

	float tangent_theta = sqrt(1.0f-pow(VN,2)) / VN;
	if(abs(tangent_theta) < 0.001f)
		return 1.0f;

	float a = sqrt(shininess*0.5f + 1.0f) / tangent_theta;

	float result = 0.0f;
	if(a < 1.6f)
		return (3.535*a + 2.181*pow(a,2)) / (1.0f + 2.276*a + 2.577*pow(a,2));
	else
		return 1.0f;
}

float G(vec3 L, vec3 V, vec3 N, float shininess)
{
	return G1(L,N, shininess) * G1(V,N, shininess);
}

vec2 getUV(vec3 L)
{
	float u = (0.5f - atan(L.y, L.x)/(2.0*pi));
	float v = acos(L.z) / pi;

	return vec2(u, v);
}