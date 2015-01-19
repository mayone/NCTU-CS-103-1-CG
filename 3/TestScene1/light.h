// light.h

#ifndef AFX_LIGHT_H_
#define AFX_LIGHT_H_

#include <string>

using namespace std;

class light {
public:
	light(const char* light_file, const int n);
	~light();
	void LoadLight(string light_file);
//private:
	int light_num;
	float **p;
	float **a;
	float **d;
	float **s;
	float *amb;
};

#endif
