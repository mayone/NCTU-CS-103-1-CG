/******************************************************************************

 * * Filename: 		main.cpp
 * * Description: 	display box, venus, and bunny
 * * 
 * * Version: 		2.0
 * * Created:		2014/11/04
 * * Revision:		none
 * * Compiler: 		g++
 * *
 * * Author: 		Wayne
 * * Organization:	CoDesign 

 ******************************************************************************/

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include "glut.h"
#endif
#include <math.h>
#include "mesh.h"
#include "light.h"
#include "view.h"
#include "scene.h"

mesh **objects;
light *light_data;
view *view_data;
scene *scene_data;

int windowSize[2];

float zoom_distance = 0.0f;
float zoom_unit = 10.0f;

float rot_x = 0.0f;
float rot_degree = 5.0f;

int selected = -1;
float diff_x;
float diff_y;

void lighting();
void display();
void draw_obj();	// draw all objects
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

int main(int argc, char** argv)
{
	light_data = new light("light.light", 2);
	view_data = new view("view.view");
	scene_data = new scene("scene.scene", 3);
	objects = new mesh*[scene_data->obj_num];
	for(int i = 0; i < scene_data->obj_num; i++) {
		objects[i] = new mesh(scene_data->obj_file_name[i]);
	}

	glutInit(&argc, argv);
	glutInitWindowPosition(view_data->viewport[0], view_data->viewport[1]);
	glutInitWindowSize(view_data->viewport[2], view_data->viewport[3]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("TestScene1");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();

	return 0;
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //²M°£¥Îcolor
	glClearDepth(1.0f);                        // Depth Buffer (´N¬Oz buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
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

	draw_obj();
	
	glutSwapBuffers();
}

void draw_obj()
{
	mesh *current;
	int lastMaterial;
	for(int i = 0; i < scene_data->obj_num; i++) {
		current = objects[i];
		lastMaterial = -1;
		glPushMatrix();
		glTranslatef(scene_data->T[i][0], scene_data->T[i][1], scene_data->T[i][2]);
		glRotatef(scene_data->Angle[i], scene_data->R[i][0], scene_data->R[i][1], scene_data->R[2][2]);
		glScalef(scene_data->S[i][0], scene_data->S[i][1], scene_data->S[i][2]);
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
				glNormal3fv(current->nList[current->faceList[i][j].n].ptr);
				glVertex3fv(current->vList[current->faceList[i][j].v].ptr);
			}
			glEnd();
		}
		glPopMatrix();
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
