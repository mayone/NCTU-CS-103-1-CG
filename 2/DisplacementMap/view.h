// view.h

#ifndef AFX_VIEW_H_
#define AFX_VIEW_H_

#include <string>

using namespace std;

class view {
public:
	view(const char* view_file);
	~view();
	void LoadView(string view_file);
//private:
	float eye[3];			// x, y, z
	float vat[3];
	float vup[3];
	float fovy;
	float dnear;
	float dfar;
	float viewport[4];	// x, y, w, h
};

#endif
