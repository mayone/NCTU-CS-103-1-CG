/******************************************************************************

 * * Filename: 		main.cpp
 * * Description: 	display the scene of world map
 * * 
 * * Version: 		1.0
 * * Created:		2014/12/13
 * * Revision:		2014/12/20
 * * Compiler: 		g++
 * *
 * * Author: 		Wayne
 * * Organization:	CoDesign 

 ******************************************************************************/

#ifdef __APPLE__
#include <freeimage.h>
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <glm/glm.hpp>	// for glm::vec2,3
#else
#include "freimage.h"
#include "glew.h"
#include "glut.h"
#endif

#include <math.h>
#include "mesh.h"
#include "light.h"
#include "view.h"
#include "scene.h"
#include "ShaderLoader.h"


unsigned *tex_objs;
GLhandleARB	ShaderProgram;
mesh **objects;
light *light_data;
view *view_data;
scene *scene_data;

int windowSize[2];

float zoom_distance = 0.0f;
float zoom_unit = 1.0f;

float rot_x = 0.0f;
float rot_degree = 5.0f;

float scaling_factor = 0.0f;
float scaling_unit = 0.5f;
GLint scaling_factor_loc;

float light_pos[2] = {0.0f, 0.0f};
float light_unit = 2.5f;

int selected = -1;
float diff_x;
float diff_y;

unsigned int DisplayListID = 1;
bool first_time = true;

GLint tangent_loc, bitangent_loc;

void LoadTexture(char* pFilename, unsigned texID);
void LoadShaders();
void lighting();
void display();
void LinkMaps();
void draw_objs();	// draw all objects
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

int main(int argc, char** argv)
{
	// load data from file
	light_data = new light("HW2.light", 1);
	view_data = new view("HW2.view");
	scene_data = new scene("HW2BumpMap.scene", 1);
	objects = new mesh*[scene_data->obj_num];
	for(int i = 0; i < scene_data->obj_num; i++) {
		objects[i] = new mesh(scene_data->obj_file_name[i]);
	}
	scene_data->tex_num = 2;
	scene_data->tex_file_name[0] = "WM.png";
	//scene_data->tex_file_name[0] = "worldColorMap.bmp";
	scene_data->tex_file_name[1] = "worldHeightMap.bmp";
	tex_objs = new unsigned[scene_data->tex_num];

	// set display properties
	glutInit(&argc, argv);
	glutInitWindowPosition(view_data->viewport[0], view_data->viewport[1]);
	glutInitWindowSize(view_data->viewport[2], view_data->viewport[3]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("WorldMapBump");

	// load and bind texture
	glewInit();
	FreeImage_Initialise();
	glGenTextures(scene_data->tex_num, tex_objs);
	for(unsigned i = 0; i < scene_data->tex_num; i++) {
		LoadTexture(scene_data->tex_file_name[i], i);
	}
	FreeImage_DeInitialise();

	// load shader programs : vertex shader, pixel shader
	LoadShaders();

	DisplayListID = glGenLists(1);	// generate id

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();

	glDeleteLists(DisplayListID, 1);

	// release allocated memory space
	delete tex_objs;
	for(int i = scene_data->obj_num - 1; i >= 0; i--) {
		delete [] objects[i];
	}
	delete [] objects;
	delete scene_data;
	delete view_data;
	delete light_data;

	return 0;
}

void LoadTexture(char* pFilename, unsigned texID)
{
	FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType(pFilename, 0), pFilename);
	FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
	int iWidth = FreeImage_GetWidth(p32BitsImage);
	int iHeight = FreeImage_GetHeight(p32BitsImage);

	int tex_type, tex_mode;
	
	tex_type = GL_TEXTURE_2D;
	tex_mode = GL_TEXTURE_2D;

	glBindTexture(tex_type, tex_objs[texID]);
	glTexImage2D(tex_mode, 0, GL_RGBA8, iWidth, iHeight, 0, GL_BGRA, 
				 GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
	glGenerateMipmap(tex_type);
	glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// i for int
	glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	float fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(tex_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);			// f for float
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	FreeImage_Unload(p32BitsImage);
	FreeImage_Unload(pImage);
	glBindTexture(tex_type, 0);
}

void LoadShaders()
{
	ShaderProgram = glCreateProgram();
	if(ShaderProgram != 0)
	{
		ShaderLoad(ShaderProgram, "BumpMapShader.vert", GL_VERTEX_SHADER);
		ShaderLoad(ShaderProgram, "BumpMapShader.frag", GL_FRAGMENT_SHADER);
	}
}

void lighting()
{
	// get ambient, diffuse, specular terms
	GLfloat light_specular[light_data->light_num][4];
	GLfloat light_diffuse[light_data->light_num][4];
	GLfloat light_ambient[light_data->light_num][4];
	GLfloat light_position[light_data->light_num][4];
	for(int i = 0; i < light_data->light_num; i++) {
		int j;
		for(j = 0; j < 3; j++) {
			light_specular[i][j] = light_data->s[i][j];
			light_diffuse[i][j] = light_data->d[i][j];
			light_ambient[i][j] = light_data->a[i][j];
			light_position[i][j] = light_data->p[i][j];
		}
		light_specular[i][j] = 1.0f;
		light_diffuse[i][j] = 1.0f;
		light_ambient[i][j] = 1.0f;
		light_position[i][j] = 1.0f;
	}
	GLfloat ambient[3];
	for(int i = 0; i < 3; i++) {
		ambient[i] = light_data->amb[i];
	}

	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);

	// set light property
	int lightN = GL_LIGHT0;
	for(int i = 0; i < light_data->light_num; i++) {
		glEnable(lightN+i);
		glLightfv(lightN+i, GL_POSITION, light_position[i]);
		glLightfv(lightN+i, GL_DIFFUSE, light_diffuse[i]);
		glLightfv(lightN+i, GL_SPECULAR, light_specular[i]);
		glLightfv(lightN+i, GL_AMBIENT, light_ambient[i]);
	}
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      // Clear using black color
	glClearDepth(1.0f);                        // Depth Buffer (namely z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear scene to black and clear z buffer

	// viewport transformation
	glViewport(0, 0, windowSize[0], windowSize[1]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(view_data->fovy, (GLfloat)windowSize[0]/(GLfloat)windowSize[1], view_data->dnear, view_data->dfar);

	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	view_data->eye[0], view_data->eye[1], view_data->eye[2],	// eye
				view_data->vat[0], view_data->vat[1], view_data->vat[2],	// center
				view_data->vup[0], view_data->vup[1], view_data->vup[2]);	// up
	// normalize
	float A = view_data->eye[0] * view_data->eye[0];
	float B = view_data->eye[1] * view_data->eye[1];
	float C = view_data->eye[2] * view_data->eye[2];
	float D = A + B + C;
	A = A / D;
	B = B / D;
	C = C / D;
	glTranslatef(A * zoom_distance, B * zoom_distance, C * zoom_distance);
	glRotatef(rot_x, 0.0, 1.0, 0.0);

	//lighting();

	glUseProgram(ShaderProgram);

	draw_objs();
	
	glutSwapBuffers();
}

void LinkMaps()
{
	GLint location;

	// 0 for colorMap
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_objs[0]);	

	location = glGetUniformLocation(ShaderProgram, "colorMap");
	if(location == -1)
		printf("Cant find texture name: colorMap\n");
	else
		glUniform1i(location, 0);

	// 1 for heightMap
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_objs[1]);

	location = glGetUniformLocation(ShaderProgram, "heightMap");
	if(location == -1)
		printf("Cant find texture name: heightMap\n");
	else
		glUniform1i(location, 1);

	// 2 for scaling_factor
	scaling_factor_loc = glGetUniformLocation(ShaderProgram, "scaling_factor");
	if(scaling_factor_loc == -1)
		printf("Cant find scaling_factor\n");
	else
		glUniform1f(scaling_factor_loc, scaling_factor);

	// 3 for light_pos
	glBindAttribLocation(ShaderProgram, 3, "light_pos");
	glVertexAttrib2f(3, light_pos[0], light_pos[1]);

	// 4, 5 for tangent, bitangent
	tangent_loc = glGetAttribLocation(ShaderProgram, "tangent");
	bitangent_loc = glGetAttribLocation(ShaderProgram, "bitangent");
}


void draw_objs()
{
	mesh *current;
	int lastMaterial;

	// for each object
	for(int objID = 0; objID < scene_data->obj_num; objID++) {
		current = objects[objID];
		lastMaterial = -1;
		glPushMatrix();
		glTranslatef(scene_data->T[objID][0], scene_data->T[objID][1], scene_data->T[objID][2]);
		glRotatef(scene_data->Angle[objID], scene_data->R[objID][0], scene_data->R[objID][1], scene_data->R[2][2]);
		glScalef(scene_data->S[objID][0], scene_data->S[objID][1], scene_data->S[objID][2]);

		LinkMaps();
		
		// draw polygons using DisplayList	
		if(first_time) {
			glNewList(DisplayListID, GL_COMPILE);	// start recording

			for(size_t i = 0; i < current->fTotal; ++i)
			{
				// set material property if this face used different material
				if(lastMaterial != current->faceList[i].m)
				{
					lastMaterial = (int)current->faceList[i].m;
					glMaterialfv(GL_FRONT, GL_AMBIENT  , current->mList[lastMaterial].Ka);
					glMaterialfv(GL_FRONT, GL_DIFFUSE  , current->mList[lastMaterial].Kd);
					glMaterialfv(GL_FRONT, GL_SPECULAR , current->mList[lastMaterial].Ks);
					glMaterialfv(GL_FRONT, GL_SHININESS, &current->mList[lastMaterial].Ns);
				}

				// get 3 vertices
				float *vertex0 = current->vList[current->faceList[i][0].v].ptr;
				float *vertex1 = current->vList[current->faceList[i][1].v].ptr;
				float *vertex2 = current->vList[current->faceList[i][2].v].ptr;
				glm::vec3 P0(vertex0[0], vertex0[1], vertex0[2]);
				glm::vec3 P1(vertex1[0], vertex1[1], vertex1[2]);
				glm::vec3 P2(vertex2[0], vertex2[1], vertex2[2]);

				// get 3 texture coordinate (u,v)
				float *texture0 = current->tList[current->faceList[i][0].v].ptr;
				float *texture1 = current->tList[current->faceList[i][1].v].ptr;
				float *texture2 = current->tList[current->faceList[i][2].v].ptr;
				glm::vec2 UV0(texture0[0], texture0[1]);
				glm::vec2 UV1(texture1[0], texture1[1]);
				glm::vec2 UV2(texture2[0], texture2[1]);

				// get edges	
				glm::vec3 Edge0 = P1 - P0;
				glm::vec3 Edge1 = P2 - P0;
				glm::vec2 Edge0uv = UV1 - UV0;
				glm::vec2 Edge1uv = UV2 - UV0;

				// calculate tangent, bitangent
				glm::vec3 tangent, bitangent;
				float cp = Edge0uv.x * Edge1uv.y - Edge1uv.x * Edge0uv.y;
				if(cp != 0.0f) {
					float gradient = 1.0f / cp;
					tangent = (Edge0 * Edge1uv.y + Edge1 * -Edge0uv.y) * gradient;
					bitangent = (Edge0 * -Edge1uv.x + Edge1 * Edge0uv.x) * gradient;
				}
				glVertexAttrib3f(tangent_loc, tangent.x, tangent.y, tangent.z);
				glVertexAttrib3f(bitangent_loc, bitangent.x, bitangent.y, bitangent.z);
				//glVertexAttrib3f(4, tangent.x, tangent.y, tangent.z);
				//glVertexAttrib3f(5, bitangent.x, bitangent.y, bitangent.z);

				glBegin(GL_TRIANGLES);
				for (size_t j = 0; j < 3; ++j)
				{
					//textex corrd. object->tList[object->faceList[i][j].t].ptr
					//glTexCoord2fv(current->tList[current->faceList[i][j].t].ptr);
					glMultiTexCoord2fv(GL_TEXTURE0, current->tList[current->faceList[i][j].t].ptr);
					//glMultiTexCoord2fv(GL_TEXTURE1, current->tList[current->faceList[i][j].t].ptr);
					glNormal3fv(current->nList[current->faceList[i][j].n].ptr);
					glVertex3fv(current->vList[current->faceList[i][j].v].ptr);
				}
				glEnd();
			}

			glEndList();	// finish recording
			first_time = false;
		}
		else {
			glCallList(DisplayListID);
		}

		// unbind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glPopMatrix();	// glFlush();
	}
}

void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
		case 'w':	zoom_distance += zoom_unit;
					printf("Zoom in\n");
					break;
		case 'a':	rot_x += rot_degree;
					printf("Rotate left\n");
					break;
		case 's':	zoom_distance -= zoom_unit;
					printf("Zoom out\n");
					break;
		case 'd':	rot_x -= rot_degree;
					printf("Rotate right\n");
					break;
		case 'e':	scaling_factor += scaling_unit;
					glLinkProgram(ShaderProgram);
					printf("Scale increase\n");
					printf("Current scaling degree: %f\n", scaling_factor);
					break;
		case 'q':	scaling_factor -= scaling_unit;
					glLinkProgram(ShaderProgram);
					printf("Scale decrease\n");
					printf("Current scaling degree: %f\n", scaling_factor);
					break;
		case 'i':	light_pos[1] += light_unit;
					glLinkProgram(ShaderProgram);
					printf("Light position move up\n");
					break;
		case 'j':	light_pos[0] -= light_unit;
					glLinkProgram(ShaderProgram);
					printf("Light position move left\n");
					break;
		case 'k':	light_pos[1] -= light_unit;
					glLinkProgram(ShaderProgram);
					printf("Light position move down\n");
					break;
		case 'l':	light_pos[0] += light_unit;
					glLinkProgram(ShaderProgram);
					printf("Light position move right\n");
					break;
	}
	if('1' <= key && key <= '9') {
		int i = key - '1';
		if(i < scene_data->obj_num) {
			selected = i;
			printf("Select object %d\n", i+1);
		}
	}
	printf("you press the key %c\n", key);
	printf("the mouse is on %d %d\n", x, y);
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	int startX, startY;
	if (state == GLUT_DOWN){
		startX = x;
		startY = y;
		if(selected >= 0) {
			diff_x = x - scene_data->T[selected][0];
			diff_y = y - scene_data->T[selected][1];
		}
	}
	else if(state == GLUT_UP) {
		printf("the mouse moves %d %d \n", x - startX, y - startY);
	}
}

void motion(int x, int y)
{
	if(selected >= 0) {
		scene_data->T[selected][0] = x - diff_x;
		scene_data->T[selected][1] = -(y - diff_y);	// Y differs in window coordinate and object coordinate
		glutPostRedisplay();
	}
	printf("the mouse is moving to %d %d\n", x, y);
}
