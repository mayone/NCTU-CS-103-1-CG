// scene.h

#ifndef AFX_SCENE_H_
#define AFX_SCENE_H_

#include <string>

using namespace std;

enum {
	NO_TEX,
	SINGLE_TEX,
	MULTI_TEX,
	CUBE_MAP,
};

class scene {
public:
	scene(const char* scene_file, const int n);
	~scene();
	void LoadScene(string scene_file);
//private:
	int tex_num;
	char **tex_file_name;
	int *tex_of_cube;
	int obj_num;
	unsigned *obj_tex_method;
	unsigned *obj_tex_ID;
	char **obj_file_name;
	float *Angle;
	float **S;
	float **R;
	float **T;
};

#endif
