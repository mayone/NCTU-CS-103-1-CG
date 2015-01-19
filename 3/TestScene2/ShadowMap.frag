//GLSL GPU Fragment Program

uniform sampler2D ShadowMap;

varying vec3 normal, lightDir, eyeDir;
varying vec4 shadowCoord;

void main()
{
	// light
	vec4 light_color;
	vec4 ambient_color;
	vec4 diffuse_color = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 specular_color = vec4(0.0, 0.0, 0.0, 0.0);

	ambient_color = (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +
					(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);

	vec3 N = normalize(normal);
	vec3 L = normalize(lightDir);

	float lambertTerm = dot(N,L);

	if(lambertTerm > 0.0)
	{
		diffuse_color = gl_LightSource[0].diffuse *
						gl_FrontMaterial.diffuse *
						lambertTerm;

		vec3 E = normalize(eyeDir);
		vec3 R = reflect(-L, N);
		float specular = pow(max(dot(R, E), 0.0),
							gl_FrontMaterial.shininess);
		specular_color = gl_LightSource[0].specular *
						gl_FrontMaterial.specular *
						specular;	
	}
	light_color = ambient_color + diffuse_color + specular_color;

	// shadow
	vec4 shadowTexCoord = shadowCoord / shadowCoord.w;	// normalize
	//shadowTexCoord = 0.5 * shadowTexCoord + 0.5;

	float tolerance = 0.005;
	float shadowMapDepth = texture2D(ShadowMap, shadowTexCoord.xy).z;
	
	float shadow = 1.0;	// no shadow
	if(shadowCoord.w > 0.0)
	{
		//shadow = shadowTexCoord.z > shadowMapDepth + tolerance ? 0.5 : 1.0;
		if(shadowTexCoord.z > shadowMapDepth + tolerance)
		{
			shadow = 0.5;
			light_color = ambient_color;
		}
	}

	gl_FragColor = shadow * light_color;
}