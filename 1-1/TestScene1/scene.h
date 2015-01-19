// scene.h

#ifndef AFX_SCENE_H_
#define AFX_SCENE_H_

#include <string>

using namespace std;

class scene {
public:
	scene(const char* scene_file, const int n);
	~scene();
	void LoadScene(string scene_file);
//private:
	int obj_num;
	char **obj_file_name;
	float *Angle;
	float **S;
	float **R;
	float **T;
};

#endif
