//GLSL GPU Vertex Program

attribute vec2 light_pos;
attribute vec3 tangent;
attribute vec3 bitangent;

varying vec3 normal, lightDir, eyeDir;
varying vec3 T_var, B_var;

void main()
{
	normal = gl_NormalMatrix * gl_Normal;

	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
	lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
	lightDir += vec3(light_pos, 0.0);

	eyeDir = -vVertex;
	
	T_var = gl_NormalMatrix * tangent;
	B_var = gl_NormalMatrix * bitangent;

	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
	gl_TexCoord[1].xy = gl_MultiTexCoord1.xy;

	//Transform the vertex (ModelViewProj matrix)
	gl_Position = ftransform();
	//gl_Position = gl_ProjectionMatrix*gl_ModelViewMatrix*gl_Vertex;
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
