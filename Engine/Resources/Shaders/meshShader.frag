#version 450 core


// Material struct.
struct Material 
{
    vec3  ambient, diffuse, specular, emission;
    float shininess, transparency;
}; 

// Light structs.
struct DirLight
{
	bool assigned;
	vec3 ambient, diffuse, specular;
	vec3 dir;
};
struct PointLight
{
	bool  assigned;
	vec3  ambient, diffuse, specular;
	float constant, linear, quadratic;
	vec3  pos;
};
struct SpotLight
{
	bool  assigned;
	vec3  ambient, diffuse, specular;
	float outerCone, innerCone;
	vec3  pos, dir;
};


// Output color.
out vec4 FragColor;

// Input pos, uv and normal.
in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in mat3 tbnMatrix;

// Textures to apply to the model.
uniform sampler2D ambientTexture,    diffuseTexture,    specularTexture,    emissionTexture,    shininessMap,    alphaMap,    normalMap;
uniform bool      useAmbientTexture, useDiffuseTexture, useSpecularTexture, useEmissionTexture, useShininessMap, useAlphaMap, useNormalMap;
vec3              ambientTexVal,     diffuseTexVal,     specularTexVal,     emissionTexVal;

// Material to apply to the model.
uniform Material material;

// Camera position.
uniform vec3 ViewPos;

// Gamma correction.
vec3 gamma = vec3(2.2);

// Maximum light counts.
#define MAX_DIR_LIGHTS 5
#define MAX_POINT_LIGHTS 20
#define MAX_SPOT_LIGHTS 20

// Light arrays.
uniform DirLight   dirLights  [MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight  spotLights [MAX_SPOT_LIGHTS];


// ----- Combination of light computations ----- //
vec3 CombineLightComputations(vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, float diff, float spec, float attenuation)
{
    vec3 ambient  = lightAmbient  * material.ambient  * ambientTexVal;
    vec3 diffuse  = lightDiffuse  * material.diffuse  * pow(diffuseTexVal, gamma)  * attenuation * diff;
    vec3 specular = lightSpecular * material.specular * specularTexVal * attenuation * spec;
    return clamp(ambient + diffuse + specular, 0, 1);
}

// ----- Directional light computation ----- //
vec3 ComputeDirLight(DirLight light, vec3 normal, vec3 viewDir, float shininess)
{
	// Skip non-assigned lights.
	if (!light.assigned)
		return vec3(0);

	// Compute lighting parameters.
	vec3  lightDir   = normalize(-light.dir);
    float diff       = max(dot(normal, lightDir), 0.0);
    vec3  reflectDir = reflect(lightDir, normal);
    float spec       = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);

	// Combine results.
    return CombineLightComputations(light.ambient, light.diffuse, light.specular, diff, spec, 1);
}

// ----- Point light computation ----- //
vec3 ComputePointLight(PointLight light, vec3 normal, vec3 viewDir, float shininess)
{
	// Skip non-assigned lights.
	if (!light.assigned)
		return vec3(0);

	// Compute lighting parameters.
	vec3  lightDir   = normalize(light.pos - FragPos);
    float diff       = max(dot(normal, lightDir), 0.0);
    vec3  reflectDir = reflect(lightDir, normal);
    float spec       = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);

	// Compute attenuation.
    float distance    = length(light.pos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
	// Combine results.
    return CombineLightComputations(light.ambient, light.diffuse, light.specular, diff, spec, attenuation);
}

// ----- Spot light computation ----- //
vec3 ComputeSpotLight(SpotLight light, vec3 normal, vec3 viewDir, float shininess)
{
	// Skip non-assigned lights.
	if (!light.assigned)
		return vec3(0);

	// Compute lighting parameters.
	vec3  lightDir = normalize(light.pos - FragPos);
	float cutoff   = dot(lightDir, -1 * light.dir);

	// Stop if the fragment isn't lit by the spot light.
	if (cutoff < light.outerCone) 
		return CombineLightComputations(light.ambient, light.diffuse, light.specular, 0, 0, 1);

	// Continue light computations.
	float diff        = max(dot(normal, lightDir), 0.0);
	vec3  reflectDir  = reflect(lightDir, normal);
	float spec        = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);
	float attenuation = clamp((cutoff - light.outerCone) / (light.innerCone - light.outerCone), 0.0, 1.0);
	
	// Combine results.
    return CombineLightComputations(light.ambient, light.diffuse, light.specular, diff, spec, attenuation);
}


void main()
{
	// Compute view dir and initialize light sum.
    vec3 viewDir = normalize(ViewPos - FragPos);
	vec3 lightSum = vec3(0, 0, 0);

	// Get normal from normal map.
	vec3 normal = Normal;
	if (useNormalMap)
	{
		normal = texture(normalMap, TexCoords).rgb * 2.0 - 1.0;
		normal = normalize(tbnMatrix * normal); // TODO: optimize this.
	}

	// Get shininess.
	float shininess = material.shininess;
	if (useShininessMap) shininess *= dot(texture(shininessMap, TexCoords), vec4(1)) / 4;

	// Get transparency.
	float transparency = material.transparency;
	if (useAlphaMap) transparency *= texture(alphaMap, TexCoords).a;
	
	// Sample all textures.
	ambientTexVal = diffuseTexVal = specularTexVal = vec3(1);
	emissionTexVal = material.emission;
	if (useAmbientTexture ) ambientTexVal  *= texture(ambientTexture,  TexCoords).rgb;
	else                    ambientTexVal  *= texture(diffuseTexture,  TexCoords).rgb;
	if (useDiffuseTexture ) diffuseTexVal  *= texture(diffuseTexture,  TexCoords).rgb;
	if (useSpecularTexture) specularTexVal *= texture(specularTexture, TexCoords).rgb;
	if (useEmissionTexture) emissionTexVal *= texture(emissionTexture, TexCoords).rgb;

	// Compute lighting for every light.
	if (length(normal) > 0.1) {
		for (int i = 0; i < MAX_DIR_LIGHTS;   i++) lightSum += ComputeDirLight  (dirLights  [i], normal, viewDir, shininess);
		for (int i = 0; i < MAX_POINT_LIGHTS; i++) lightSum += ComputePointLight(pointLights[i], normal, viewDir, shininess);
		for (int i = 0; i < MAX_SPOT_LIGHTS;  i++) lightSum += ComputeSpotLight (spotLights [i], normal, viewDir, shininess);
	}
	else {
		lightSum = diffuseTexVal;
	}

	FragColor = vec4(lightSum + emissionTexVal, transparency);
    FragColor.rgb = pow(FragColor.rgb, 1.0/gamma);
}
