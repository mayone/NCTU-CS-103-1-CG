/******************************************************************************

 * * Filename: 		main.cpp
 * * Description: 	display the TwoToys
 * * 
 * * Version: 		1.0
 * * Created:		2015/1/1
 * * Revision:		2015/1/14
 * * Compiler: 		g++
 * *
 * * Author: 		Wayne
 * * Organization:	CoDesign

 ******************************************************************************/

#ifdef __APPLE__
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <GLM/glm.hpp>
#else
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

GLuint FBO;		// Framebuffer Object

GLuint depthTexture;
GLuint shadowMapUniform;

int windowSize[2];

float zoom_distance = 0.0f;
float zoom_unit = 1.0f;

float rot_x = 0.0f;
float rot_degree = 5.0f;

float scaling_factor = 0.0f;
float scaling_unit = 0.5f;

GLint light_pos_loc;
float light_pos[2] = {0.0f, 0.0f};
float light_unit = 1.0f;

enum display_mode {
	SCENE_ONLY,
	MINI_MAP,
	MAP_ONLY
};
int mode = SCENE_ONLY;

int selected = -1;
float diff_x;
float diff_y;

void GenTextures();
void LoadShaders();
void lighting();
void display();
void GenShadow();
void SaveLightMVP();	// ModelViewProjection
void RenScene();
void SaveCameraViewInverse();
void drawObjects();
void ShowMiniMap();
void ShowShadowMap();
void SaveTRS_start(	float Tx, float Ty, float Tz,
					float Ra, float Rx, float Ry, float Rz,
					float Sx, float Sy, float Sz);
void SaveTRS_end();
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

int main(int argc, char** argv)
{
	// load data from file
	light_data = new light("TwoToys.light", 1);
	view_data = new view("TwoToys.view");
	scene_data = new scene("TwoToys.scene", 3);
	objects = new mesh*[scene_data->obj_num];
	for(int i = 0; i < scene_data->obj_num; i++) {
		objects[i] = new mesh(scene_data->obj_file_name[i]);
	}
	scene_data->tex_num = 0;
	tex_objs = new unsigned[scene_data->tex_num];

	windowSize[0] = view_data->viewport[2];
	windowSize[1] = view_data->viewport[3];

	// set display properties
	glutInit(&argc, argv);
	glutInitWindowPosition(view_data->viewport[0], view_data->viewport[1]);
	glutInitWindowSize(windowSize[0], windowSize[1]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("TwoToys");

	// load and bind texture and buffer
	glewInit();
	GenTextures();

	// load shader programs : vertex shader, pixel shader
	LoadShaders();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();

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

void GenTextures()
{
	GLenum FBOstatus;
	int iWidth = windowSize[0];
	int iHeight = windowSize[1];

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, iWidth, iHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// GL_LINEAR does not make sense for depth texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// remove artefact on the edges of the shadowmap
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// generate Framebuffer Object
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	// check FBO status
	FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE)
		printf("GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n");

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LoadShaders()
{
	ShaderProgram = glCreateProgram();
	if(ShaderProgram != 0)
	{
		ShaderLoad(ShaderProgram, "ShadowMap.vert", GL_VERTEX_SHADER);
		ShaderLoad(ShaderProgram, "ShadowMap.frag", GL_FRAGMENT_SHADER);
	}
	shadowMapUniform = glGetUniformLocation(ShaderProgram, "ShadowMap");
	if(shadowMapUniform == -1)
		printf("Cant find texture name: ShadowMap\n");
	light_pos_loc = glGetAttribLocation(ShaderProgram, "light_pos");
}

void lighting()
{
	// get ambient, diffuse, specular terms
	GLfloat light_specular[light_data->light_num][4];
	GLfloat light_diffuse[light_data->light_num][4];
	GLfloat light_ambient[light_data->light_num][4];
	GLfloat light_position[light_data->light_num][4];
	for(int i = 0; i < light_data->light_num; i++)
	{
		int j;
		for(j = 0; j < 3; j++)
		{
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
	for(int i = 0; i < 3; i++)
	{
		ambient[i] = light_data->amb[i];
	}

	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);

	// set light property
	GLuint lightN = GL_LIGHT0;
	for(int i = 0; i < light_data->light_num; i++)
	{
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// Clear using black color
	glClearDepth(1.0f);							// Depth Buffer (namely z buffer) Setup

	glEnable(GL_DEPTH_TEST);					// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);						// The Type Of Depth Test To Do
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	glEnable(GL_CULL_FACE);	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

	// Pass1: render depth to texture
	GenShadow();
	// save modelview, projection matrices of light into texture, also add a biais
	SaveLightMVP();

	// Pass2: render to screen
	RenScene();
	if(mode == MINI_MAP)
	{
		ShowMiniMap();
	}
	if(mode == MAP_ONLY)
	{
		ShowShadowMap();
	}

	glutSwapBuffers();
}

void GenShadow()
{
	// render to texture
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// use the fixed pipeline to render to the depth buffer(Z-Buffer)
	glUseProgram(0);

	// disable color rendering, only write to Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// clear depth buffer(Z-Buffer) in FBO
	glClear(GL_DEPTH_BUFFER_BIT);

	// viewport transformation
	glViewport(0, 0, windowSize[0], windowSize[1]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)windowSize[0]/(GLfloat)windowSize[1], 10.0, 60.0);

	// viewing and modeling transformation	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	light_data->p[0][0]+light_pos[0], light_data->p[0][1]+light_pos[1]+1, light_data->p[0][2]+0.00000000000000001,
				view_data->vat[0], view_data->vat[1], view_data->vat[2],
				view_data->vup[0], view_data->vup[1], view_data->vup[2]);
	// culling switching, render only backface to avoid self-shadowing
	//glCullFace(GL_BACK);

	drawObjects();
}

void SaveLightMVP()
{
	static float modelView[4*4];
	static float projection[4*4];
	const GLfloat bias[4*4] = 
	{
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	};

	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);	// get view
	glGetFloatv(GL_PROJECTION_MATRIX, projection);	// get projection

	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE1);

	glLoadIdentity();	
	glLoadMatrixf(bias);
	glMultMatrixf(projection);
	glMultMatrixf(modelView);
}

void RenScene()
{
	// render to scene
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// use the shader code to render to the scene
	glUseProgram(ShaderProgram);

	// enable color rendering
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// clear scene to black and clear depth buffer(Z-Buffer)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	A = sqrt(A / D);
	B = sqrt(B / D);
	C = sqrt(C / D);
	glTranslatef(A * zoom_distance, B * zoom_distance, C * zoom_distance);
	glRotatef(rot_x, 0.0, 1.0, 0.0);
	
	lighting();

	// culling switching, render only frontface
	glCullFace(GL_BACK);

	glUniform1i(shadowMapUniform, 1);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	drawObjects();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SaveCameraViewInverse()
{
	glm::mat4 view;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);	// get view
	glm::mat4 viewInverse = glm::inverse(view);

	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE1);

	glLoadIdentity();	
	glMultMatrixf(&viewInverse[0][0]);
}

void drawObjects()
{
	mesh *current;
	int lastMaterial;

	// for each object
	for(int objID = 0; objID < scene_data->obj_num; objID++)
	{
		current = objects[objID];
		lastMaterial = -1;
		glPushMatrix();

		glTranslatef(scene_data->T[objID][0], scene_data->T[objID][1], scene_data->T[objID][2]);
		glRotatef(scene_data->Angle[objID], scene_data->R[objID][0], scene_data->R[objID][1], scene_data->R[objID][2]);
		glScalef(scene_data->S[objID][0], scene_data->S[objID][1], scene_data->S[objID][2]);

		SaveTRS_start(	scene_data->T[objID][0], scene_data->T[objID][1], scene_data->T[objID][2],
						scene_data->Angle[objID], scene_data->R[objID][0], scene_data->R[objID][1], scene_data->R[objID][2],
						scene_data->S[objID][0], scene_data->S[objID][1], scene_data->S[objID][2]);	

		// draw polygons
		for(size_t i = 0; i < current->fTotal; i++)
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
			glBegin(GL_TRIANGLES);
			for(size_t j = 0; j < 3; j++)
			{
				//textex corrd. object->tList[object->faceList[i][j].t].ptr
				//glTexCoord2fv(current->tList[current->faceList[i][j].t].ptr);
				//glMultiTexCoord2fv(GL_TEXTURE1, current->tList[current->faceList[i][j].t].ptr);
				glNormal3fv(current->nList[current->faceList[i][j].n].ptr);
				glVertex3fv(current->vList[current->faceList[i][j].v].ptr);
			}
			glEnd();
		}

		SaveTRS_end();

		glPopMatrix();	// glFlush();
	}
}

void ShowMiniMap()
{
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-windowSize[0]/2, windowSize[0]/2, -windowSize[1]/2, windowSize[1]/2, 1, 20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTranslatef(windowSize[0]/5.85, windowSize[1]/5.85, -1);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);glVertex3f(0, 0, 0);
	glTexCoord2d(1, 0);glVertex3f(windowSize[0]/3, 0, 0);
	glTexCoord2d(1, 1);glVertex3f(windowSize[0]/3, windowSize[1]/3, 0);
	glTexCoord2d(0, 1);glVertex3f(0, windowSize[1]/3, 0);

	glEnd();
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ShowShadowMap()
{
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-windowSize[0]/2, windowSize[0]/2, -windowSize[1]/2, windowSize[1]/2, 1, 20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTranslatef(-windowSize[0]/2, -windowSize[1]/2, -1);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);glVertex3f(0, 0, 0);
	glTexCoord2d(1, 0);glVertex3f(windowSize[0], 0, 0);
	glTexCoord2d(1, 1);glVertex3f(windowSize[0], windowSize[1], 0);
	glTexCoord2d(0, 1);glVertex3f(0, windowSize[1], 0);

	glEnd();
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SaveTRS_start(	float Tx, float Ty, float Tz,
					float Ra, float Rx, float Ry, float Rz,
					float Sx, float Sy, float Sz)
{
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE1);
	glPushMatrix();
	glTranslatef(Tx, Ty, Tz);
	glRotatef(Ra, Rx, Ry, Rz);
	glScalef(Sx, Sy, Sz);
}

void SaveTRS_end()
{
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
	GenTextures();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
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
		case 'm':	printf("display mode change!\n");
					if(mode == SCENE_ONLY) mode = MINI_MAP;
					else if(mode == MINI_MAP) mode = SCENE_ONLY;
					break;
		case 'n':	printf("display mode change!\n");
					if(mode == SCENE_ONLY) mode = MAP_ONLY;
					else if(mode == MAP_ONLY) mode = SCENE_ONLY;
					break;
		case 'i':	light_pos[1] += light_unit;
					glVertexAttrib2fv(light_pos_loc, light_pos);
					printf("Light position move up\n");
					break;
		case 'j':	light_pos[0] -= light_unit;
					glVertexAttrib2fv(light_pos_loc, light_pos);
					printf("Light position move left\n");
					break;
		case 'k':	light_pos[1] -= light_unit;
					glVertexAttrib2fv(light_pos_loc, light_pos);
					printf("Light position move down\n");
					break;
		case 'l':	light_pos[0] += light_unit;
					glVertexAttrib2fv(light_pos_loc, light_pos);
					printf("Light position move right\n");
					break;
		case 27:	printf("Program exit\n");	// ESC
					exit(0);
					break;
	}
	if('1' <= key && key <= '9')
	{
		int i = key - '1';
		if(i < scene_data->obj_num)
		{
			selected = i;
			printf("Select object %d\n", i+1);
		}
		else
		{
			selected = -1;
		}
	}
	printf("you press the key %c\n", key);
	printf("the mouse is on %d %d\n", x, y);
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	int startX, startY;
	if(state == GLUT_DOWN)
	{
		startX = x;
		startY = y;
		if(selected >= 0)
		{
			diff_x = x - scene_data->T[selected][0];
			diff_y = y - scene_data->T[selected][1];
		}
	}
	else if(state == GLUT_UP)
	{
		printf("the mouse moves %d %d \n", x - startX, y - startY);
	}
}

void motion(int x, int y)
{
	if(selected >= 0)
	{
		scene_data->T[selected][0] = x - diff_x;
		scene_data->T[selected][1] = -(y - diff_y);	// Y differs in window coordinate and object coordinate
		glutPostRedisplay();
	}
	printf("the mouse is moving to %d %d\n", x, y);
}
