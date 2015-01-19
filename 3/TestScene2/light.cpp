// light.cpp

#include "light.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

light::light(const char* light_file, const int n)
{
	light_num = n;
	p = new float*[light_num];
	a = new float*[light_num];
	d = new float*[light_num];
	s = new float*[light_num];
	for(int i = 0; i < light_num; i++) {
		p[i] = new float[3];	// x, y, z
		a[i] = new float[3];	// r, g, b
		d[i] = new float[3];	// r, g, b
		s[i] = new float[3];	// r, g, b
	}
	amb = new float[3];
	LoadLight((string)light_file);
}

light::~light()
{
	for(int i = light_num - 1; i >= 0; i--) {
		delete [] p[i];
		delete [] a[i];
		delete [] d[i];
		delete [] s[i];
	}
	delete [] p;
	delete [] a;
	delete [] d;
	delete [] s;
	delete [] amb;
}

void light::LoadLight(string light_file)
{
	FILE *light_fptr;
	char token[100];

	light_fptr = fopen(light_file.c_str(),"r");

	if(!light_fptr) {
		cout<< string("Can not open light File \"") << light_file << "\" !" << endl;
		return;
	}

	cout << endl << light_file << endl;
	
	int i = 0;
	while(!feof(light_fptr)) {
		token[0] = '\0';
		fscanf(light_fptr,"%s", token);
		if(!strcmp(token, "light")) {
			for(int j = 0; j < 3; j++) {
				fscanf(light_fptr, "%f", &p[i][j]); 
			}
			for(int j = 0; j < 3; j++) {
				fscanf(light_fptr, "%f", &a[i][j]);
			}
			for(int j = 0; j < 3; j++) {
				fscanf(light_fptr, "%f", &d[i][j]);
			}
			for(int j = 0; j < 3; j++) {
				fscanf(light_fptr, "%f", &s[i][j]);
			}
			i++;
		}
		if(!strcmp(token, "ambient")) {
			for(int j = 0; j < 3; j++) {
				fscanf(light_fptr, "%f", &amb[j]);
			}
		}
	}
	cout << "light sources: " << i << endl;
}