//GLSL GPU Vertex Program

//P' = P + (N * df * uf)
//uf = user scaling factor
//df = displacement factor
//convert RGB to grey value
//df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z


//varying vec3 normal, lightDir, eyeDir;
attribute float scaling_factor;
uniform sampler2D heightMap;

void main()
{
	//normal = gl_NormalMatrix * gl_Normal;

	//vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
	//lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
	//lightDir += vec3(light_pos, 0.0);

	vec4 newVertexPos;
	vec4 dv;
	float df;

	gl_TexCoord[0].xy = gl_MultiTexCoord1.xy;
	dv = texture2D(heightMap, gl_MultiTexCoord1.xy).rgba;
	df = dv.r + dv.g + dv.b;

	newVertexPos = vec4(gl_Normal * df * scaling_factor, 0.0) + gl_Vertex;

	//Transform the vertex (ModelViewProj matrix)
	//gl_Position = ftransform();
	//gl_Position = gl_ProjectionMatrix*gl_ModelViewMatrix*gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
}
