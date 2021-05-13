//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <chrono>
#include <algorithm>
#include <iostream>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"

#define MAX_INTERVAL 500

#ifdef _WIN32
#define print sprintf_s
#else
#define print sprintf
#endif

bool GraphicalUI::stopTrace = false;
GraphicalUI* GraphicalUI::pUI = NULL;
const char* GraphicalUI::traceWindowLabel = "Raytraced Image";
bool TraceUI::m_debug = false;

//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ((GraphicalUI*)(o->parent()->user_data()));
}

//--------------------------------- Callback Functions --------------------------------------------
void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	static char* lastFile = 0;
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			print(buf, "Ray <%s>", newfile);
			stopTracing();	// terminate the previous rendering
		} else print(buf, "Ray <Not Loaded>");

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
			pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}
}

void GraphicalUI::cb_load_cubemap(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);
	pUI->m_cubeMapChooser->show();
}

void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	pUI = whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	pUI = (GraphicalUI *)(o->user_data());

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_exit3(Fl_Widget* o, void* v) 
{
	pUI = (GraphicalUI *)(o->user_data());

	// terminate the rendering
	//	stopTracing();
	pUI->m_debuggingWindow->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project for CS384g.");
}

void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());

	// terminate the rendering so we don't get crashes
	stopTracing();

	pUI->m_nSize=int(((Fl_Slider *)o)->value());
	int width = (int)(pUI->getSize());
	int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow(width, height);
	// Need to call traceSetup before trying to render
//	pUI->raytracer->setReady(false);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_thresholdSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nThreshold=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_blockSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nBlockSize=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_refreshSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->refreshInterval=clock_t(((Fl_Slider *)o)->value()) ;
}

void GraphicalUI::cb_threadSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_threads = (int)(((Fl_Slider*)o)->value());
}

void GraphicalUI::cb_aaSamplesSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nSuperSamples=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_aaThresholdSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nAaThreshold=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_kdTreeDepthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nTreeDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_kdLeafSizeSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nLeafSize=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_filterSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_nFilterWidth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_displayDebuggingInfo)
	  {
	    pUI->m_debuggingWindow->show();
	    pUI->m_debug = true;
	  }
	else
	  {
	    pUI->m_debuggingWindow->hide();
	    pUI->m_debug = false;
	  }
}

void GraphicalUI::cb_ssCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_smoothshade = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_shCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_shadows = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_bfCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_backface = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_aaCheckButton(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	pUI->m_antiAlias = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_antiAlias) 
	{
		pUI->m_aaSamplesSlider->activate();
		pUI->m_aaThreshSlider->activate();
	}
	else
	{
		pUI->m_aaSamplesSlider->deactivate();
		pUI->m_aaThreshSlider->deactivate();
	}
}

void GraphicalUI::cb_kdCheckButton(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	pUI->m_kdTree = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_kdTree) 
	{
		pUI->m_treeDepthSlider->activate();
		pUI->m_leafSizeSlider->activate();
	}
	else
	{
		pUI->m_treeDepthSlider->deactivate();
		pUI->m_leafSizeSlider->deactivate();
	}
}

void GraphicalUI::cb_cubeMapCheckButton(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	pUI->m_usingCubeMap = (((Fl_Check_Button*)o)->value() == 1 && pUI->getCubeMap() != nullptr);
	if (pUI->m_usingCubeMap) 
		pUI->m_filterSlider->activate();
	else {
		pUI->m_filterSlider->deactivate();
		((Fl_Check_Button*)o)->value(0);
	}
}

void GraphicalUI::cb_render(Fl_Widget* o, void* v) {

	char buffer[256];

	pUI = (GraphicalUI*)(o->user_data());
	stopTrace = false;
	if (!pUI->raytracer->sceneLoaded()){
		char buf[256];
	
		if (pUI->raytracer->loadScene("")) {
			print(buf, "Ocean");
			stopTracing();	// terminate the previous rendering
		} else print(buf, "Ocean <Not Loaded>");

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}

		int width = pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		int origPixels = width * height;
		pUI->m_traceGlWindow->resizeWindow(width, height);
		pUI->m_traceGlWindow->show();
		clock_t startTime, now, prev, traceTime;
		startTime = now = prev = clock();
		auto t_start = std::chrono::high_resolution_clock::now();
		auto t_now = t_start;
		auto t_elapsed = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
		pUI->raytracer->traceImage(width, height);
		clock_t intervalMS = pUI->refreshInterval * 100;
		while (!pUI->raytracer->checkRender())
		{
			// check for input and refresh view every so often while tracing
			std::this_thread::sleep_for(std::chrono::milliseconds(std::min(intervalMS, (clock_t)MAX_INTERVAL)));
			now = clock();
			traceTime = now - startTime;
			t_now = std::chrono::high_resolution_clock::now();
			t_elapsed = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
			if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
			{
				print(buffer, "Time: %.2f sec, Rays: %u", t_elapsed, TraceUI::getCount());
				pUI->m_traceGlWindow->label(buffer);
				pUI->m_traceGlWindow->refresh();
				prev = now;
			}
			// look for input and refresh window
			Fl::wait(0);			
			if (Fl::damage()) { Fl::flush(); }
		}
		traceTime = clock() - startTime;
		t_now = std::chrono::high_resolution_clock::now();
		auto t_trace = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
		int imageRays = TraceUI::resetCount();
		print(buffer, "Time: %.2f sec, Rays: %u, Aa: none", t_trace, imageRays);
		pUI->m_traceGlWindow->label(buffer);
		pUI->m_traceGlWindow->refresh();
		if (pUI->aaSwitch() && !stopTrace)
		{
			clock_t aaStart, aaTime;
			auto t_aaStart = std::chrono::high_resolution_clock::now();
			auto t_total = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
			aaStart = now = prev = clock();
			int aaPixels = pUI->raytracer->aaImage();
			while (!pUI->raytracer->checkRender())
			{
				// check for input and refresh view every so often while tracing
				std::this_thread::sleep_for(std::chrono::milliseconds(std::min(intervalMS, (clock_t)MAX_INTERVAL)));
				now = clock();
				aaTime = now - aaStart;
				t_now = std::chrono::high_resolution_clock::now();
				t_elapsed = std::chrono::duration<double, std::ratio<1>>(t_now - t_aaStart).count();
				t_total = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
				if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
				{
					print(buffer, "Trace: %.2f, Aa: %.2f, Total: %.2f, aaRays: %d",
					      t_trace, t_elapsed, t_total, TraceUI::getCount()); 
					pUI->m_traceGlWindow->label(buffer);
					pUI->m_traceGlWindow->refresh();
					prev = now;
				}
				// look for input and refresh window
				Fl::wait(0);			
				if (Fl::damage()) { Fl::flush(); }
			}
			aaTime = clock() - aaStart;
			t_now = std::chrono::high_resolution_clock::now();
			t_elapsed = std::chrono::duration<double, std::ratio<1>>(t_now - t_aaStart).count();
			t_total = std::chrono::duration<double, std::ratio<1>>(t_now - t_start).count();
			int aaRays = TraceUI::resetCount();
			print(buffer, "Trace: %.2f, Aa: %.2f, Total: %.2f, Rays: %u, %u, %u",
			      t_trace, t_elapsed, t_total, imageRays, aaRays, imageRays + aaRays);
			pUI->m_traceGlWindow->label(buffer);
			pUI->m_traceGlWindow->refresh();
		}
/*
		pUI->raytracer->setThreshold(pUI->getThreshold());

		bool zOrder = false;
		if (zOrder) {
			int th = height;
			int tw = width;
			while (th != 0) {
				bits++;
				th = th >> 1;
			}      
			while (tw != 0) {
				bits++;
				tw = tw >> 1;
			}      
		}
					int frac = min(100,(int)((double)(b_width*b_height*num_blocks)/(double)(width*height)*100.0));
					// update the window label
					print(buffer, "%d%% of %s", frac, traceWindowLabel);
					pUI->m_traceGlWindow->label(buffer);
				}

				int ix = 0;
				int iy = 0;
				if (zOrder) {
					int mask = 1;
					for (int nn = 0; nn < bits; nn++) {
						ix += ((nextBlock & mask) >> nn);
						mask = mask << 1;
						iy += ((nextBlock & mask) >> (nn + 1));
						mask = mask << 1;
					}
					ix *= b_width;
					iy *= b_height;
					nextBlock++;
				}
				else {
					ix = ii;
					iy = jj;
				}
				// data has changed, update on next refresh
				pUI->m_debuggingWindow->m_debuggingView->setDirty();
*/
	
}

void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	stopTracing();
}

int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer(tracer);
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File", 0, 0, 0, FL_SUBMENU },
	{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
	{ "&Load Cubemap...", FL_ALT + 'c', (Fl_Callback *)GraphicalUI::cb_load_cubemap },
	{ "&Save Image...", FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
	{ "&Exit", FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
	{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
	{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
	{ 0 },

	{ 0 }
};

void GraphicalUI::stopTracing()
{
	stopTrace = true;
	pUI->raytracer->stopTrace = true;

	// Wait for the trace to finish (simple synchronization)
	while(!pUI->raytracer->checkRender()) Fl::wait();
//	while(!doneTrace)	Fl::wait();
}

GraphicalUI::GraphicalUI() : refreshInterval(10) {
	// init.
	m_threads = std::max(std::thread::hardware_concurrency(), (unsigned) 1);

	m_mainWindow = new Fl_Window(100, 40, 450, 459, "Ray <Not Loaded>");
	m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
	// install menu bar
	m_menubar = new Fl_Menu_Bar(0, 0, 440, 25);
	m_menubar->menu(menuitems);

	// set up "render" button
	m_renderButton = new Fl_Button(360, 37, 70, 25, "&Render");
	m_renderButton->user_data((void*)(this));
	m_renderButton->callback(cb_render);

	// set up "stop" button
	m_stopButton = new Fl_Button(360, 65, 70, 25, "&Stop");
	m_stopButton->user_data((void*)(this));
	m_stopButton->callback(cb_stop);

	// install depth slider
	m_depthSlider = new Fl_Value_Slider(10, 40, 180, 20, "Recursion Depth");
	m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_depthSlider->type(FL_HOR_NICE_SLIDER);
	m_depthSlider->labelfont(FL_COURIER);
	m_depthSlider->labelsize(12);
	m_depthSlider->minimum(0);
	m_depthSlider->maximum(10);
	m_depthSlider->step(1);
	m_depthSlider->value(m_nDepth);
	m_depthSlider->align(FL_ALIGN_RIGHT);
	m_depthSlider->callback(cb_depthSlides);

	// install blocksize slider
	m_blockSlider = new Fl_Value_Slider(10, 65, 180, 20, "Blocksize");
	m_blockSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_blockSlider->type(FL_HOR_NICE_SLIDER);
	m_blockSlider->labelfont(FL_COURIER);
	m_blockSlider->labelsize(12);
	m_blockSlider->minimum(2);
	m_blockSlider->maximum(64);
	m_blockSlider->step(1);
	m_blockSlider->value(m_nBlockSize);
	m_blockSlider->align(FL_ALIGN_RIGHT);
	m_blockSlider->callback(cb_blockSlides);

	// install threshold slider
	m_thresholdSlider = new Fl_Value_Slider(10, 90, 180, 20, "Threshold (x 0.001)");
	m_thresholdSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_thresholdSlider->type(FL_HOR_NICE_SLIDER);
	m_thresholdSlider->labelfont(FL_COURIER);
	m_thresholdSlider->labelsize(12);
	m_thresholdSlider->minimum(0);
	m_thresholdSlider->maximum(1000);
	m_thresholdSlider->step(1);
	m_thresholdSlider->value(m_nThreshold);
	m_thresholdSlider->align(FL_ALIGN_RIGHT);
	m_thresholdSlider->callback(cb_thresholdSlides);

	// install size slider
	m_sizeSlider = new Fl_Value_Slider(10, 115, 180, 20, "Screen Size");
	m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_sizeSlider->type(FL_HOR_NICE_SLIDER);
	m_sizeSlider->labelfont(FL_COURIER);
	m_sizeSlider->labelsize(12);
	m_sizeSlider->minimum(64);
	m_sizeSlider->maximum(1024);
	m_sizeSlider->step(2);
	m_sizeSlider->value(m_nSize);
	m_sizeSlider->align(FL_ALIGN_RIGHT);
	m_sizeSlider->callback(cb_sizeSlides);

	// install refresh interval slider
	m_refreshSlider = new Fl_Value_Slider(10, 140, 180, 20, "Screen Refresh Interval (0.1 sec)");
	m_refreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_refreshSlider->type(FL_HOR_NICE_SLIDER);
	m_refreshSlider->labelfont(FL_COURIER);
	m_refreshSlider->labelsize(12);
	m_refreshSlider->minimum(1);
	m_refreshSlider->maximum(300);
	m_refreshSlider->step(1);
	m_refreshSlider->value(refreshInterval);
	m_refreshSlider->align(FL_ALIGN_RIGHT);
	m_refreshSlider->callback(cb_refreshSlides);

	// install threads slider
	m_refreshSlider = new Fl_Value_Slider(10, 165, 180, 20, "Threads");
	m_refreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_refreshSlider->type(FL_HOR_NICE_SLIDER);
	m_refreshSlider->labelfont(FL_COURIER);
	m_refreshSlider->labelsize(12);
	m_refreshSlider->minimum(1);
	m_refreshSlider->maximum(32);
	m_refreshSlider->step(1);
	m_refreshSlider->value(m_threads);
	m_refreshSlider->align(FL_ALIGN_RIGHT);
	m_refreshSlider->callback(cb_threadSlides);

	// install aasamples slider
	m_aaSamplesSlider = new Fl_Value_Slider(95, 205, 180, 20, "Pixel Samples\non Each Direction");
	m_aaSamplesSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_aaSamplesSlider->type(FL_HOR_NICE_SLIDER);
	m_aaSamplesSlider->labelfont(FL_COURIER);
	m_aaSamplesSlider->labelsize(12);
	m_aaSamplesSlider->minimum(1);
	m_aaSamplesSlider->maximum(8);
	m_aaSamplesSlider->step(1);
	m_aaSamplesSlider->value(m_nSuperSamples);
	m_aaSamplesSlider->align(FL_ALIGN_RIGHT);
	m_aaSamplesSlider->callback(cb_aaSamplesSlides);
	if (!m_antiAlias) m_aaSamplesSlider->deactivate();

	// install aathreshold slider
	m_aaThreshSlider = new Fl_Value_Slider(95, 237, 180, 20, "Supersample\nThreshold (x 0.001)");
	m_aaThreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_aaThreshSlider->type(FL_HOR_NICE_SLIDER);
	m_aaThreshSlider->labelfont(FL_COURIER);
	m_aaThreshSlider->labelsize(12);
	m_aaThreshSlider->minimum(0);
	m_aaThreshSlider->maximum(1000);
	m_aaThreshSlider->step(1);
	m_aaThreshSlider->value(m_nAaThreshold);
	m_aaThreshSlider->align(FL_ALIGN_RIGHT);
	m_aaThreshSlider->callback(cb_aaThresholdSlides);
	if (!m_antiAlias) m_aaThreshSlider->deactivate();

	// set up antialias checkbox
	m_aaCheckButton = new Fl_Check_Button(10, 221, 75, 20, "Antialias");
	m_aaCheckButton->user_data((void*)(this));
	m_aaCheckButton->callback(cb_aaCheckButton);
	m_aaCheckButton->value(m_antiAlias);

	// install kdmaxdepth slider
	m_treeDepthSlider = new Fl_Value_Slider(95, 277, 180, 20, "Max Depth");
	m_treeDepthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_treeDepthSlider->type(FL_HOR_NICE_SLIDER);
	m_treeDepthSlider->labelfont(FL_COURIER);
	m_treeDepthSlider->labelsize(12);
	m_treeDepthSlider->minimum(1);
	m_treeDepthSlider->maximum(30);
	m_treeDepthSlider->step(1);
	m_treeDepthSlider->value(m_nTreeDepth);
	m_treeDepthSlider->align(FL_ALIGN_RIGHT);
	m_treeDepthSlider->callback(cb_kdTreeDepthSlides);
	if (!m_kdTree) m_treeDepthSlider->deactivate();

	// install kdleafsize slider
	m_leafSizeSlider = new Fl_Value_Slider(95, 309, 180, 20, "Target Leaf Size");
	m_leafSizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_leafSizeSlider->type(FL_HOR_NICE_SLIDER);
	m_leafSizeSlider->labelfont(FL_COURIER);
	m_leafSizeSlider->labelsize(12);
	m_leafSizeSlider->minimum(1);
	m_leafSizeSlider->maximum(100);
	m_leafSizeSlider->step(1);
	m_leafSizeSlider->value(m_nLeafSize);
	m_leafSizeSlider->align(FL_ALIGN_RIGHT);
	m_leafSizeSlider->callback(cb_kdLeafSizeSlides);
	if (!m_kdTree) m_leafSizeSlider->deactivate();

	// install cubemap filter width slider
	m_filterSlider = new Fl_Value_Slider(95, 349, 180, 20, "Filter Width");
	m_filterSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_filterSlider->type(FL_HOR_NICE_SLIDER);
	m_filterSlider->labelfont(FL_COURIER);
	m_filterSlider->labelsize(12);
	m_filterSlider->minimum(1);
	m_filterSlider->maximum(17);
	m_filterSlider->step(1);
	m_filterSlider->value(m_nFilterWidth);
	m_filterSlider->align(FL_ALIGN_RIGHT);
	m_filterSlider->callback(cb_filterSlides);
	if (!m_usingCubeMap) m_filterSlider->deactivate();

	// set up kdTree checkbox
	m_kdCheckButton = new Fl_Check_Button(10, 293, 80, 20, "K-d Tree");
	m_kdCheckButton->user_data((void*)(this));
	m_kdCheckButton->callback(cb_kdCheckButton);
	m_kdCheckButton->value(m_kdTree);

	// set up cubeMap checkbox
	m_cubeMapCheckButton = new Fl_Check_Button(10, 349, 80, 20, "CubeMap");
	m_cubeMapCheckButton->user_data((void*)(this));
	m_cubeMapCheckButton->callback(cb_cubeMapCheckButton);
	m_cubeMapCheckButton->value(m_usingCubeMap);
	if (cubemap)
		m_cubeMapCheckButton->activate();
	else m_cubeMapCheckButton->deactivate();

	// set up smoothshade checkbox
	m_ssCheckButton = new Fl_Check_Button(10, 389, 110, 20, "Smoothshade");
	m_ssCheckButton->user_data((void*)(this));
	m_ssCheckButton->callback(cb_ssCheckButton);
	m_ssCheckButton->value(m_smoothshade);

	// set up shadows checkbox
	m_shCheckButton = new Fl_Check_Button(140, 389, 80, 20, "Shadows");
	m_shCheckButton->user_data((void*)(this));
	m_shCheckButton->callback(cb_shCheckButton);
	m_shCheckButton->value(m_shadows);

	// set up backfacing checkbox
	m_bfCheckButton = new Fl_Check_Button(240, 389, 110, 20, "Backface Cull");
	m_bfCheckButton->user_data((void*)(this));
	m_bfCheckButton->callback(cb_bfCheckButton);
	m_bfCheckButton->value(m_backface);

	// set up debugging display checkbox
	m_debuggingDisplayCheckButton = new Fl_Check_Button(10, 419, 140, 20, "Debugging display");
	m_debuggingDisplayCheckButton->user_data((void*)(this));
	m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
	m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);

	m_mainWindow->callback(cb_exit2);
	m_mainWindow->when(FL_HIDE);
	m_mainWindow->end();

	// set up cubemap chooser
	std::unique_ptr<TextureMap> cubeFace[6];

	//D:/Projects/Graphics/ray/build/bin/Release/images/Daylight Box_Right_posx.bmp
	//D:/Projects/Graphics/ray/build/bin/Release/images/Daylight Box_Right_posx.bmp
	try {
		cubeFace[0].reset(new TextureMap("../../../src/images/Daylight Box_Right_posx.bmp"));
	} catch (TextureMapException &xcpt) {
		std::cerr << xcpt.message() << std::endl;
		std::string msg("Error: could not open file: ");
		msg.append("../../../src/images/Daylight Box_Right_posx.bmp");
	}
	cubeFace[1].reset(new TextureMap("../../../src/images/Daylight Box_Left_negx.bmp"));
	cubeFace[2].reset(new TextureMap("../../../src/images/Daylight Box_Top_posy.bmp"));
	cubeFace[3].reset(new TextureMap("../../../src/images/Daylight Box_Bottom_negy.bmp"));
	cubeFace[4].reset(new TextureMap("../../../src/images/Daylight Box_Front_posz.bmp"));
	cubeFace[5].reset(new TextureMap("../../../src/images/Daylight Box_Back_negz.bmp"));

	if (!getCubeMap()) {
		setCubeMap(new CubeMap());
	}

	for (int i = 0; i < 6; i++)
		getCubeMap()->setNthMap(i, cubeFace[i].release());
	useCubeMap(true);

	m_cubeMapChooser = new CubeMapChooser();
	m_cubeMapChooser->setCaller(this);

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, traceWindowLabel);
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);

	// debugging view
	m_debuggingWindow = new DebuggingWindow();
	//	Fl_Window::default_callback((Fl_Window*)m_debuggingWindow, cb_exit3);
	//	((Fl_Window*)m_debuggingWindow)->callback(cb_exit3);

	// char buf[256];
	
	// if (raytracer->loadScene("")) {
	// 	print(buf, "Ray <%s>", "");
	// 	stopTracing();	// terminate the previous rendering
	// } else print(buf, "Ray <Not Loaded>");

	// m_mainWindow->label(buf);
	// m_debuggingWindow->m_debuggingView->setDirty();

	// m_debuggingWindow->m_debuggingView->resetCamera();

	// m_debuggingWindow->redraw();
}

#endif
