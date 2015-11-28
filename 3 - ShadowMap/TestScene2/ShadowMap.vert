//GLSL GPU Vertex Program

attribute vec2 light_pos;

varying vec3 normal, lightDir, eyeDir;
varying vec4 shadowCoord;

void main()
{
	// calculate the normal for the vertex, in world coordinates (multiply by gl_NormalMatrix)
	normal = gl_NormalMatrix * gl_Normal;

	// calculate the light direction
	vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	lightDir = vec3(gl_LightSource[0].position.xyz - vertexPos);
	lightDir += vec3(light_pos, 0.0);

    eyeDir = -vertexPos;

	shadowCoord = gl_TextureMatrix[1] * gl_Vertex;

	// transform the vertex (ModelViewProj matrix)
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = ftransform();
}