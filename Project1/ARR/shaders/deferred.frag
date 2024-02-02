#version 430 core

struct MainLight
{
	uint type;				// 0, Directional=0, Point=1, Spotlight=2
	vec3 position;
	vec3 direction;
	vec3 attenuationConstants;
	float innerAngle;
	float outerAngle;
	float fallOut;
};

uniform uint mode;			// 0: total, 1: world, 2: normal, 3: diffuse, 4: specular

uniform MainLight mainLight;

layout (binding=0) uniform sampler2D gWorldPosition;
layout (binding=1) uniform sampler2D gNormalVector;
layout (binding=2) uniform sampler2D gDiffuse;
layout (binding=3) uniform sampler2D gSpecular;

uniform vec2 screenSize;

in vec3 eye;
out vec4 FragColor;

vec2 uv;
vec3 GetColor();
vec3 LightDirection(vec3 position);

void main()
{
	uv = gl_FragCoord.xy / screenSize;
	
	vec3 tNormal = texture(gNormalVector, uv).rgb;

	if(!(length(tNormal) != 0))
		discard;
		
	vec3 resultColor = GetColor();
    FragColor.xyz = resultColor;
}

vec3 GetColor()
{	
	vec3 tPosition = texture(gWorldPosition, uv).rgb;
	vec3 tNormal = texture(gNormalVector, uv).rgb;
	vec3 tDiffuse = texture(gDiffuse, uv).rgb;
	vec4 tSpecular = texture(gSpecular, uv);
	float distance = length(mainLight.position - tPosition);
	float att = min(1.0 / (mainLight.attenuationConstants.x + (mainLight.attenuationConstants.y * distance) 
		+ (mainLight.attenuationConstants.z * (distance*distance))), 1.0);

	if(mode == 0)
	{
		vec3 lightVec = LightDirection(tPosition);
		vec3 eyeVec = eye - tPosition;
		vec3 N = normalize(tNormal);
		vec3 L = normalize(lightVec);
	    vec3 V = normalize(eyeVec);
	    vec3 H = normalize(L+V);
	    float NL = max(dot(N,L),0.0);	// diffuse
	    float NV = max(dot(N,V),0.0);	// 
	    float HN = max(dot(H,N),0.0);
	    float shininess = tSpecular.a;

	    vec3 diffsue = NL * tDiffuse;
		vec3 specular = tSpecular.rgb * pow(HN, shininess);

	    vec3 result = vec3(0.0f);
	    if(mainLight.type == 0)
	    {
	    	result = diffsue + specular;
	    }
	    else if(mainLight.type == 1)
	    {
	    	result = (diffsue + specular) * att;
	    }
	    else if(mainLight.type == 2)
	    {
	    	float spotEffect = 0;
	    	vec3 LD = normalize(mainLight.direction);
	    	float angle = dot(LD, -lightVec);
	    	float phi =  cos(radians(mainLight.outerAngle));
	    	float theta =  cos(radians(mainLight.innerAngle));

	    	if(angle < phi)
				spotEffect = 0;
			else if(angle > theta)
				spotEffect = 1;
			else
				spotEffect = pow((angle-phi)/(theta-phi), mainLight.fallOut);

			result = att * spotEffect * (diffsue + specular);
	    }

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
	else if(mode ==4 )
	{
		return tSpecular.rgb;
	}
}

vec3 LightDirection(vec3 position)
{
	if(mainLight.type == 0)
		return normalize(-mainLight.direction);

	return normalize(mainLight.position - position);
}