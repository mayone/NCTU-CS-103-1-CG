// scene.cpp

#include "scene.h"
#include <iostream>

#define NUM_TEXTURE 100
#define STRLEN 100

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

scene::scene(const char* scene_file, const int n)
{
	tex_num = 0;
	tex_file_name = new char*[NUM_TEXTURE];
	tex_of_cube = new int[NUM_TEXTURE];
	obj_num = n;
	obj_tex_method = new unsigned[obj_num];
	obj_tex_ID = new unsigned[obj_num];
	obj_file_name = new char*[obj_num];
	S = new float*[obj_num];
	Angle = new float[obj_num];
	R = new float*[obj_num];
	T = new float*[obj_num];
	for(int i = 0; i < NUM_TEXTURE; i++) {
		tex_file_name[i] = new char[STRLEN];
	}
	for(int i = 0; i < obj_num; i++) {
		obj_file_name[i] = new char[STRLEN];
		S[i] = new float[3];	// Sx, Sy, Sz
		R[i] = new float[3];	// Rx, Ry, Rz
		T[i] = new float[3];	// Tx, Ty, Tz
	}
	LoadScene((string)scene_file);
}

scene::~scene()
{
	for(int i = NUM_TEXTURE - 1; i >= 0; i--) {
		delete [] tex_file_name[i];
	}
	for(int i = obj_num - 1; i >= 0; i--) {
		delete [] obj_file_name[i];
		delete [] S[i];
		delete [] R[i];
		delete [] T[i];
	}
	delete [] tex_file_name;
	delete [] tex_of_cube;
	delete [] obj_tex_method;
	delete [] obj_tex_ID;
	delete [] obj_file_name;
	delete [] Angle;
	delete [] S;
	delete [] R;
	delete [] T;
}

void scene::LoadScene(string scene_file)
{
	FILE *scene_fptr;
	char token[100];

	scene_fptr = fopen(scene_file.c_str(),"r");

	if(!scene_fptr) {
		cout<< string("Can not open scene File \"") << scene_file << "\" !" << endl;
		return;
	}

	cout << endl << scene_file << endl;
	
	int i = 0, j;
	unsigned method;
	unsigned texID = 0;
	while(!feof(scene_fptr)) {
		token[0] = '\0';
		fscanf(scene_fptr,"%s", token);

		if(!strcmp(token, "no-texture")) {
			method = NO_TEX;
		}
		if(!strcmp(token, "single-texture")) {
			method = SINGLE_TEX;
			texID = tex_num;
			tex_of_cube[tex_num] = -1;
			fscanf(scene_fptr, "%s", tex_file_name[tex_num++]);
		}
		if(!strcmp(token, "multi-texture")) {
			method = MULTI_TEX;
			texID = tex_num;
			for(j = 0; j < 2; j++) {
				tex_of_cube[tex_num] = -1;
				fscanf(scene_fptr, "%s", tex_file_name[tex_num++]);
			}
		}
		if(!strcmp(token, "cube-map")) {
			method = CUBE_MAP;
			texID = tex_num;
			for(j = 0; j < 6; j++) {
				tex_of_cube[tex_num] = j;
				fscanf(scene_fptr, "%s", tex_file_name[tex_num++]);
			}
		}

		if(!strcmp(token, "model")) {
			obj_tex_method[i] = method;
			obj_tex_ID[i] = texID;
			fscanf(scene_fptr, "%s", obj_file_name[i]);
			for(int j = 0; j < 3; j++) {
				fscanf(scene_fptr, "%f", &S[i][j]); 
			}
			fscanf(scene_fptr, "%f", &Angle[i]);
			for(int j = 0; j < 3; j++) {
				fscanf(scene_fptr, "%f", &R[i][j]); 
			}
			for(int j = 0; j < 3; j++) {
				fscanf(scene_fptr, "%f", &T[i][j]); 
			}
			i++;
		}
	}
	cout << "tex number: " << tex_num << endl;
	cout << "obj scene: " << i << endl;
}