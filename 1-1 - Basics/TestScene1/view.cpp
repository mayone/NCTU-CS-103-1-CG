// view.cpp

#include "view.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

view::view(const char* view_file)
{
	LoadView((string)view_file);
}

view::~view()
{}

void view::LoadView(string view_file)
{
	FILE *view_fptr;
	char token[100];

	view_fptr = fopen(view_file.c_str(),"r");

	if(!view_fptr) {
		cout<< string("Can not open view File \"") << view_file << "\" !" << endl;
		return;
	}

	cout << endl << view_file << endl;
	
	while(!feof(view_fptr)) {
		token[0] = '\0';
		fscanf(view_fptr,"%s", token);

		if(!strcmp(token, "eye")) {
			for(int i = 0; i < 3; i++) {
				fscanf(view_fptr, "%f", &eye[i]); 
			}
		}
		if(!strcmp(token, "vat")) {
			for(int i = 0; i < 3; i++) {
				fscanf(view_fptr, "%f", &vat[i]);
			}
		}
		if(!strcmp(token, "vup")) {
			for(int i = 0; i < 3; i++) {
				fscanf(view_fptr, "%f", &vup[i]);
			}
		}
		if(!strcmp(token, "fovy")) {
			fscanf(view_fptr, "%f", &fovy);
		}
		if(!strcmp(token, "dnear")) {
			fscanf(view_fptr, "%f", &dnear);
		}
		if(!strcmp(token, "dfar")) {
			fscanf(view_fptr, "%f", &dfar);
		}
		if(!strcmp(token, "viewport")) {
			for(int i = 0; i < 4; i++) {
				fscanf(view_fptr, "%f", &viewport[i]);
			}
		}
	}
	cout << "view loading completed" << endl;
}