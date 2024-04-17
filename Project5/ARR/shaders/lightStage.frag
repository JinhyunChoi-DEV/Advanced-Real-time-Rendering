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
uniform sampler2D gDepth;
uniform sampler2D gDeviceDepth;
uniform sampler2D shadowMap;
uniform sampler2D backgroundTexture; 
uniform sampler2D irradMap;
uniform sampler2D aoMap;
uniform mat4 shadowMatrix;
uniform vec2 screenSize;
uniform float alpha;
uniform float near; 
uniform float far;
uniform float exposureControl;
uniform bool onlyAO;
uniform float threshold;
uniform int npSelected;

uniform HammersleyBlock
{
	int N;
	float hammersley[2*100];
} HB;

in vec3 eye;
out vec4 FragColor;

const float pi = 3.141592653589793;

vec2 uv;
vec3 GetColor();
vec3 cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3);
float CaluateShadow(vec4 worldPos);

float D(vec3 N, vec3 H, float shininess);
vec3 F(vec3 L, vec3 H, vec3 Ks);
float G1(vec3 v, vec3 H, float shininess);
float G(vec3 L, vec3 V, vec3 H, float shininess);
vec2 getUV(vec3 L);
float GetNon_Photorealistic();

vec2 distortUV(vec2 uv, float amount) 
{    
    float noiseX = sin(uv.y * 10.0) * amount;
    float noiseY = cos(uv.x * 10.0) * amount;
    return uv + vec2(noiseX, noiseY);
}


const float S_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
const float S_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

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
	    float shininess = 1.0 - tSpecular.a;
	    vec3 H_ = normalize(L+V);

	    vec3 R = 2.0 * dot(N,V) * N - V;
	    vec3 A = normalize(cross(vec3(0,0,1), R));
	    vec3 B = normalize(cross(R,A));

	    vec3 diffuseColor = texture2D(irradMap, getUV(N)).rgb * (Kd / pi);
	    vec3 specularColor = vec3(0.0);
	    if(abs(shininess) < 0.0001f )
	    	shininess = 0.001;

	    for(int i = 0; i < HB.N; ++i)
	    {
	    	float ksai_1 = HB.hammersley[i*2];
	    	float ksai_2 = HB.hammersley[i*2+1];
	    	float theta = atan((shininess*sqrt(ksai_2)) / sqrt(1.0-ksai_2));

	    	float u = ksai_1;
	    	float v = theta / pi;
	    	vec3 direction = vec3(0);
	    	direction.x = cos(2.0*pi*(0.5-u))*sin(pi*v);
	    	direction.y = sin(2.0*pi*(0.5-u))*sin(pi*v);
	    	direction.z = cos(pi*v);

	    	vec3 omegaK = normalize(vec3(direction.x * A + direction.y * B + direction.z * R));
	    	vec3 H = normalize(omegaK + V);

	    	float level = 0.5 * log2((screenSize.x * screenSize.y) / HB.N) - 0.5*log2(D(N,H,shininess));
	    	vec2 LSM = getUV(omegaK);
	    	vec3 pixel = textureLod(backgroundTexture, LSM, level).rgb;

	    	vec3 nom = G(omegaK, V, H, shininess) * F(omegaK, H, Ks);
	    	float denom = 4.0 * max(dot(omegaK, N), 0.1) * max(dot(V,N), 0.01)+ 0.01;

	    	specularColor += (nom / denom) * pixel * cos(theta);
	    }
	    specularColor /= HB.N;
	    vec3 result = diffuseColor + specularColor;
	   	result = pow((exposureControl*result)/((exposureControl*result)+vec3(1,1,1)), vec3(1.0/2.2f));
	
	   	float AO = texture2D(aoMap, uv).r;
	   	if(onlyAO)
	   	{
	   		return vec3(AO);
	   	}
	   	else
	   	{
	   		if(npSelected == 0)
	   			return result * AO;
	   		if(npSelected == 1)
	   			return vec3(GetNon_Photorealistic());
	   		if(npSelected == 2)
	    		return result * AO * GetNon_Photorealistic();
	   	}
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

float D(vec3 N, vec3 H, float shininess)
{
	float a2 = shininess * shininess;
	float NH = max(dot(N,H), 0.0);

	return a2 / (pi*pow((NH*NH)*(a2-1.0) +1.0,2.0));
}

vec3 F(vec3 L, vec3 H, vec3 Ks)
{
	float LH = max(dot(L,H), 0.0);

	return Ks + (vec3(1.0) - Ks) * pow((1.0 - LH), 5.0);
}

float G1(vec3 v, vec3 H, float shininess)
{
	float vH = max(dot(v, H), 0);
	float tan_theta = sqrt(1.0 - (vH*vH)) / vH;
	float a2 = shininess * shininess;

	return 2.0 / (1.0 + sqrt(1.0 + (a2 * tan_theta*tan_theta)));
}

float G(vec3 L, vec3 V, vec3 H, float shininess)
{
	return G1(L,H, shininess) * G1(V,H, shininess);
}

vec2 getUV(vec3 L)
{
	float u = (0.5 - atan(L.y, L.x)/(2.0*pi));
	float v = acos(L.z) / pi;

	return vec2(u, v);
}

float GetNon_Photorealistic()
{
	vec2 sizeDepth = textureSize(gDeviceDepth, 0);
	vec2 sizeNormal = textureSize(gNormalVector, 0);

	float edgeX_d = 0.0;
	float edgeY_d = 0.0;
	float edgeX_n = 0.0;
	float edgeY_n = 0.0;

	for(int i = -1; i <= 1; ++i)
	{
		for(int j = -1; j <=1; ++j)
		{
			vec2 offset_depth = vec2(float(i), float(j)) / sizeDepth; 
			vec2 offset_normal = vec2(float(i), float(j)) / sizeNormal;

			vec2 neighborTex_depth = uv + offset_depth;
			vec2 neighborTex_normal = uv + offset_normal;

			vec3 color_depth = texture2D(gDeviceDepth, neighborTex_depth).rgb;
			vec3 color_normal = texture2D(gNormalVector, neighborTex_normal).rgb;

			edgeX_d += color_depth.r *  S_x[(i+1) * 3 + (j+1)];
			edgeY_d += color_depth.r *  S_y[(i+1) * 3 + (j+1)];

			edgeX_n += color_normal.r *  S_x[(i+1) * 3 + (j+1)];
			edgeY_n += color_normal.r *  S_y[(i+1) * 3 + (j+1)];
		}
	}

	float edgeStrengh_d = sqrt(edgeX_d * edgeX_d + edgeY_d * edgeY_d);
	float edgeStrengh_n = sqrt(edgeX_n * edgeX_n + edgeY_n * edgeY_n);

	edgeStrengh_d = edgeStrengh_d > threshold ? 1.0 : 0.0;
	edgeStrengh_n = edgeStrengh_n > threshold ? 1.0 : 0.0;

	return 1.0 - (edgeStrengh_n + edgeStrengh_d);
}