/******************************************************************************

 * * Filename: 		main.cpp
 * * Description: 	display the scene of chess
 * * 
 * * Version: 		1.0
 * * Created:		2014/11/12
 * * Revision:		2014/11/14
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


unsigned *tex_objs;
mesh **objects;
light *light_data;
view *view_data;
scene *scene_data;

int windowSize[2];

float zoom_distance = 0.0f;
float zoom_unit = 1.0f;

float rot_x = 0.0f;
float rot_degree = 5.0f;

int selected = -1;
float diff_x;
float diff_y;

void LoadTexture(char* pFilename, unsigned texID);
void lighting();
void display();
void draw_objs();	// draw all objects
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

int main(int argc, char** argv)
{
	// load data from file
	light_data = new light("Chess.light", 1);
	view_data = new view("Chess.view");
	scene_data = new scene("Chess.scene", 34);
	objects = new mesh*[scene_data->obj_num];
	for(int i = 0; i < scene_data->obj_num; i++) {
		objects[i] = new mesh(scene_data->obj_file_name[i]);
	}
	tex_objs = new unsigned[scene_data->tex_num];

	// set display properties
	glutInit(&argc, argv);
	glutInitWindowPosition(view_data->viewport[0], view_data->viewport[1]);
	glutInitWindowSize(view_data->viewport[2], view_data->viewport[3]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Chess");

	// load and bind texture
	glewInit();
	FreeImage_Initialise();
	glGenTextures(scene_data->tex_num, tex_objs);
	for(int i = 0; i < scene_data->tex_num; i++) {
		LoadTexture(scene_data->tex_file_name[i], i);
	}
	FreeImage_DeInitialise();

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

void LoadTexture(char* pFilename, unsigned texID)
{
	FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType(pFilename, 0), pFilename);
	FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
	int iWidth = FreeImage_GetWidth(p32BitsImage);
	int iHeight = FreeImage_GetHeight(p32BitsImage);

	int tex_type, tex_mode;
	switch(scene_data->tex_of_cube[texID]) {
		case -1:
			tex_type = GL_TEXTURE_2D;
			tex_mode = GL_TEXTURE_2D;
			break;
		case 0:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			break;
		case 1:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			texID -= 1;
			break;
		case 2:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			texID -= 2;
			break;
		case 3:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			texID -= 3;
			break;
		case 4:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			texID -= 4;
			break;
		case 5:
			tex_type = GL_TEXTURE_CUBE_MAP;
			tex_mode = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			texID -= 5;
			break;
		default:
			break;
	}

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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      // Black color
	glClearDepth(1.0f);                        // Depth Buffer (´N¬Oz buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//³o¦æ§âµe­±²M¦¨¶Â¦â¨Ã¥B²M°£z buffer

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

	lighting();

	draw_objs();
	
	glutSwapBuffers();
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
		glRotatef(scene_data->Angle[objID], scene_data->R[objID][0], scene_data->R[objID][1], scene_data->R[objID][2]);
		glScalef(scene_data->S[objID][0], scene_data->S[objID][1], scene_data->S[objID][2]);

		// bind textures
		int tex_n;
		switch(scene_data->obj_tex_method[objID]) {
			case NO_TEX:
				break;
			case SINGLE_TEX:
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, tex_objs[scene_data->obj_tex_ID[objID]]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case MULTI_TEX:
				tex_n = GL_TEXTURE0;
				for(int tex_offset = 0; tex_offset < 2; tex_offset++) {
					glActiveTexture(tex_n + tex_offset);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, tex_objs[scene_data->obj_tex_ID[objID]+tex_offset]);
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				}
				break;
			case CUBE_MAP:
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_GEN_R);
				glEnable(GL_TEXTURE_CUBE_MAP);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	// homework required
				break;
			default:
				break;
		}

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

			glBegin(GL_TRIANGLES);
			for (size_t j = 0; j < 3; ++j)
			{
				//textex corrd. object->tList[object->faceList[i][j].t].ptr
				switch(scene_data->obj_tex_method[objID]) {
					case NO_TEX:
						break;
					case SINGLE_TEX:
						glTexCoord2fv(current->tList[current->faceList[i][j].t].ptr);
						break;
					case MULTI_TEX:
						glMultiTexCoord2fv(GL_TEXTURE0, current->tList[current->faceList[i][j].t].ptr);
						glMultiTexCoord2fv(GL_TEXTURE1, current->tList[current->faceList[i][j].t].ptr);
						break;
					case CUBE_MAP:
						break;
					default:
						break;
				}
				glNormal3fv(current->nList[current->faceList[i][j].n].ptr);
				glVertex3fv(current->vList[current->faceList[i][j].v].ptr);
			}
			glEnd();
		}

		// unbind textures
		switch(scene_data->obj_tex_method[objID]) {
			case NO_TEX:
				break;
			case SINGLE_TEX:
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
				break;
			case MULTI_TEX:
				tex_n = GL_TEXTURE0;
				for(int tex_offset = 2-1; tex_offset >= 0; tex_offset--) {
					glActiveTexture(tex_n + tex_offset);
					glDisable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				break;
			case CUBE_MAP:
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);
				glDisable(GL_TEXTURE_CUBE_MAP);
				break;
			default:
				break;
		}
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
	}
	if('1' <= key && key <= '9') {
		int i = key - '1';
		if(i < scene_data->obj_num) {
			selected = i;
			printf("Select object %d\n", i+1);
			/*
			view_data->vat[0] = scene_data->T[i][0];
			view_data->vat[1] = scene_data->T[i][1];
			view_data->vat[2] = scene_data->T[i][2];
			printf("Look at object %d\n", i+1);
			*/
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
			//scene_data->T[i][0]; 
			//scene_data->T[i][1];
			//scene_data->T[i][2];
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
