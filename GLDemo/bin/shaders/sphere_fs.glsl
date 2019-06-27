#version 330 core
out vec4 FragColor;

//holds the textures
struct Material {
	sampler2D diffuse;
	sampler2D normal;
	sampler2D specular;
	float shininess;
};

//holds the properties for the lighting
struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

//data received from the vertex shader
in VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} fs_in;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool isSun;

void main() {
	if (!isSun) { //if this planetoid isn't the sun, skip the lighting calculations and just apply the diffuse texture
		vec3 color = texture(material.diffuse, fs_in.TexCoords).rgb;
		vec3 normal = texture(material.normal, fs_in.TexCoords).rgb;
		normal = normalize(normal * 2.0 - 1.0);

		//apply ambient lighting
		vec3 ambient = light.ambient * color;

		//apply diffuse lighting
		vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 diffuse = light.diffuse * diff * color;

		//apply specular lighting
		vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = 0.0;
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		vec3 specular = light.specular * spec * texture(material.specular, fs_in.TexCoords).rgb;

		//apply lighting attenuation 
		float distance = length(light.position - fs_in.FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		//calculate sum of all lights and output the resulting fragment color
		vec3 lighting = ambient + diffuse + specular;
		FragColor = vec4(lighting, 1.0);
	} else {
		FragColor = texture(material.diffuse, fs_in.TexCoords);
	}
}