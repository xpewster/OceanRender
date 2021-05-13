//
// originalview.h
//
// The header file of the view for rendered image
//

#ifndef __TRACEGLWINDOW_H__
#define __TRACEGLWINDOW_H__

#include <FL/Fl.H>

#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/glu.h>

#include "../RayTracer.h"

class TraceGLWindow : public Fl_Gl_Window
{
public:
	TraceGLWindow(int x, int y, int w, int h, const char *l);
	void draw();
	void show();
	int handle(int event);

	void refresh();

	void resizeWindow(int width, int height);

	void saveImage(char *iname);

	void setRayTracer(RayTracer *tracer);

private:
	RayTracer *raytracer;
	int m_nWindowWidth, m_nWindowHeight;
	int m_nDrawWidth, m_nDrawHeight;
};

#endif // __TRACE_GL_WINDOW_H__

