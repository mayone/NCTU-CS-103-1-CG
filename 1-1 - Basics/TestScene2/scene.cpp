// scene.cpp

#include "scene.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

scene::scene(const char* scene_file, const int n)
{
	obj_num = n;
	obj_file_name = new char*[obj_num];
	S = new float*[obj_num];
	Angle = new float[obj_num];
	R = new float*[obj_num];
	T = new float*[obj_num];
	for(int i = 0; i < obj_num; i++) {
		obj_file_name[i] = new char[100];
		S[i] = new float[3];	// Sx, Sy, Sz
		R[i] = new float[3];	// Rx, Ry, Rz
		T[i] = new float[3];	// Tx, Ty, Tz
	}
	LoadScene((string)scene_file);
}

scene::~scene()
{
	for(int i = obj_num - 1; i >= 0; i--) {
		delete [] obj_file_name[i];
		delete [] S[i];
		delete [] R[i];
		delete [] T[i];
	}
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
	
	int i = 0;
	while(!feof(scene_fptr)) {
		token[0] = '\0';
		fscanf(scene_fptr,"%s", token);

		if(!strcmp(token, "model")) {
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
	cout << "obj scene: " << i << endl;
}