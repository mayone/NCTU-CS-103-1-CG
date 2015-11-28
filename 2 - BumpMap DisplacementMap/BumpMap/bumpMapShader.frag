//GLSL GPU Fragment Program

uniform sampler2D colorMap;
uniform sampler2D heightMap;
uniform float scaling_factor;

varying vec3 normal, lightDir, eyeDir;
varying vec3 T_var, B_var;

void main()
{
	vec4 final_color = 
	(gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + 
	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);

	vec3 T = normalize(T_var);
	vec3 B = normalize(B_var);

	// C: center, R: right, U: up
	vec4 pC = texture2D(heightMap, gl_TexCoord[0].xy).rgba;
	vec4 pR = texture2D(heightMap, gl_TexCoord[0].xy + vec2(1.0/1024.0, 0.0)).rgba;
	vec4 pU = texture2D(heightMap, gl_TexCoord[0].xy + vec2(0.0, 1.0/1024.0)).rgba;
	float hC = pC.r + pC.g + pC.b;
	float hR = pR.r + pR.g + pR.b;
	float hU = pU.r + pU.g + pU.b;
	float Hx = hR - hC;
	float Hy = hU - hC;

	vec3 new_normal = normal - (Hx * T + Hy * B) * scaling_factor;

	vec3 N = normalize(new_normal);
	vec3 L = normalize(lightDir);

	vec4 diffuse = texture2D(colorMap, gl_TexCoord[0].xy).rgba;

	float lambertTerm = dot(N,L);

	if(lambertTerm > 0.0)
	{
		/*
		final_color += gl_LightSource[0].diffuse * 
		               gl_FrontMaterial.diffuse * 
					   lambertTerm;
		*/
		final_color += diffuse * lambertTerm;

		vec3 E = normalize(eyeDir);
		vec3 R = reflect(-L, N);
		float specular = pow(max(dot(R, E), 0.0), 
		                 gl_FrontMaterial.shininess );
		final_color += gl_LightSource[0].specular * 
		               gl_FrontMaterial.specular * 
					   specular;	
	}
	gl_FragColor = final_color;
	//gl_FragColor = final_color * texture2D(colorMap, gl_TexCoord[0].xy).rgba;
}
