// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "scene/water_constant.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>


using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y)
{
	// Clear out the ray cache in the scene for debugging purposes,
	if (TraceUI::m_debug)
	{
		scene->clearIntersectCache();		
	}

	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
	scene->getCamera().rayThrough(x,y,r);
	double dummy;
	glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), 0, dummy);
	ret = glm::clamp(ret, 0.0, 1.0);
	return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j)
{
	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return col;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	col = trace(x, y);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	isect i;
	glm::dvec3 colorC;
#if VERBOSE
	std::cerr << "== current depth: " << depth << std::endl;
#endif

	if(scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		//cout << "recursion depth: " << depth << "\n";	

		if (std::isnan(i.getT()))
			std::cout << "BAD T!\n" << i.getN() << std::endl; 
		
		const Material& m = i.getMaterial();
		colorC = m.shade(scene.get(), r, i);

		if (r.isUnderwater()){
			float radiance = caustics_map.get_radiance_square(r.at(i.getT()), 0.01f); 
			colorC += glm::vec3(1)*(pow(std::min(85.0f*radiance, 1.0f)+0.5f, 2.0f)-0.5f);
			//draw photons:
			// float p_radiance = caustics_map.get_radiance_square(r.at(i.getT()), 0.0005); 
			// colorC += glm::vec3(1.0,0.0,0.0)*pow(p_radiance*100.0f*photon_resolution, 1.0f);
		}

		if (depth < traceUI->getDepth()){
			/* Reflection */
			ray r2(r.at(i.getT()), glm::normalize(glm::reflect<3, double>(r.getDirection(), i.getN())), glm::dvec3(1,1,1), ray::REFLECTION);
			
			double dummy;
			if (glm::length(m.kr(i)) > 0)
				colorC += m.kr(i)*traceRay(r2, thresh, depth+1, dummy);

			/* Refraction */
			if (glm::length(m.kt(i)) > 0){
				double n_i, n_t;
				glm::dvec3 norm = i.getN();
				if (glm::dot(r.getDirection(), i.getN()) <= 0){
					n_i = 1.0;
					n_t = m.index(i);
				}
				else {
					norm = -norm;
					n_i = m.index(i);
					n_t = 1.0;
				}
				
				double cosine = glm::dot(norm, r.getDirection());
				double root = 1.0 - (n_i/n_t)*(n_i/n_t)*(1.0 - cosine*cosine);
				if (root >= 0){ // not total internal reflection
					glm::dvec3 dir = glm::normalize(
							((n_i/n_t)*cosine - sqrt(root))*norm
							-(n_i/n_t)*r.getDirection());
					//std::cout << glm::to_string(r.getDirection()) << ", " << glm::to_string(dir) << "| " << glm::to_string(i.getN()) << ", " << (n_i/n_t) << std::endl;
					ray r3(r.at(i.getT()), dir, glm::dvec3(1,1,1), ray::REFRACTION, false);
					// if (std::isnan(glm::normalize(glm::refract(r.getDirection(), norm, n_i/n_t))[0])){
					// 	glm::dvec3 asdf = glm::normalize(glm::refract(r.getDirection(), norm, n_i/n_t));
					// }
					colorC += m.kt(i)*traceRay(r3, thresh, depth+1, dummy);
				}
				// glm::dvec3 dir = glm::refract(r.getDirection(), norm, n_i/n_t);
				// if (dir != glm::dvec3(0))
				// 	dir = normalize(dir);
	
				// ray r3(r.at(i.getT()), dir, glm::dvec3(1,1,1), ray::REFRACTION);
				// if (glm::dot(r.getDirection(), r3.getDirection()) != 0.0){ // not total internal reflection
				// 	colorC += m.kt(i)*traceRay(r3, thresh, depth+1, dummy);
				// }
				else { //TIR
					ray r4(r.at(i.getT()), glm::normalize(glm::reflect<3, double>(r.getDirection(), i.getN())), glm::dvec3(1,1,1), ray::REFRACTION);
					colorC += m.kt(i)*traceRay(r4, thresh, depth+1, dummy);
				}
			}
		}

		if (r.isUnderwater()){
			double time_uw = i.getT();
			//atten *= time_uw/1.0 * glm::dvec3(0, 0.12, 0.3);
			if (time_uw > water::water_size)
				colorC = water::water_deep_color;
			else
				colorC = colorC * (1.0 - (time_uw/water::water_size)*(time_uw/water::water_size)) + water::water_deep_color*(time_uw/water::water_size)*(time_uw/water::water_size);

			double step_size = 0.005;
			float ss_radiance = 0.0f;
			// for(double _t = 0; (r.at(_t).x > water::water_size/-2.0f && r.at(_t).x < water::water_size/2.0f 
			// 				&& r.at(_t).z > water::water_size/-2.0f && r.at(_t).z < water::water_size/2.0f) ; _t += step_size){
			for(double _t = 0; _t < i.getT(); _t += step_size){
				ss_radiance += ss_map.get_radiance_rect(r.at(_t), 0.005f, 0.05f, 0.005f) * _t/water::water_size;
			}
			colorC += glm::vec3(1)*0.15f*std::min(std::max(pow(std::min(10000.0f*ss_radiance, 1.0f)+0.5f, 1.0f)-0.5f, 0.0f), 1.0f);
		}
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		//
		// FIXME: Add CubeMap support here.
		// TIPS: CubeMap object can be fetched from traceUI->getCubeMap();
		//       Check traceUI->cubeMap() to see if cubeMap is loaded
		//       and enabled.
		if	(traceUI->cubeMap()) {
			if (r.isUnderwater()){
				colorC = water::water_deep_color;
				double step_size = 0.005;
				float ss_radiance = 0.0f;
				for(double _t = 0; _t < water::water_size; _t += step_size){
					ss_radiance += ss_map.get_radiance_rect(r.at(_t), 0.005f, 0.05f, 0.005f) * _t/water::water_size;
				}
				colorC += glm::vec3(1)*0.15f*std::min(std::max(pow(std::min(10000.0f*ss_radiance, 1.0f)+0.5f, 1.0f)-0.5f, 0.0f), 1.0f);
			}
			else
				colorC = traceUI->getCubeMap()->getColor(r);
		}
		else{
			colorC = glm::dvec3(0);
		}
	}
#if VERBOSE
	std::cerr << "== depth: " << depth+1 << " done, returning: " << colorC << std::endl;
#endif
	return colorC;
}

RayTracer::RayTracer()
	: scene(nullptr), buffer(0), thresh(0), buffer_width(0), buffer_height(0), m_bBufferReady(false), ws(0.0)
{
}

RayTracer::~RayTracer()
{
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer.data();
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char* fn)
{
	scene.reset(ws.getScene());
	// ifstream ifs(fn);
	// if( !ifs ) {
	// 	string msg( "Error: couldn't read scene file " );
	// 	msg.append( fn );
	// 	traceUI->alert( msg );
	// 	return false;
	// }

	// // Strip off filename, leaving only the path:
	// string path( fn );
	// if (path.find_last_of( "\\/" ) == string::npos)
	// 	path = ".";
	// else
	// 	path = path.substr(0, path.find_last_of( "\\/" ));

	// // Call this with 'true' for debug output from the tokenizer
	// Tokenizer tokenizer( ifs, false );
	// Parser parser( tokenizer, path );
	// try {
	// 	scene.reset(parser.parseScene());
	// }
	// catch( SyntaxErrorException& pe ) {
	// 	traceUI->alert( pe.formattedMessage() );
	// 	return false;
	// } catch( ParserException& pe ) {
	// 	string msg( "Parser: fatal exception " );
	// 	msg.append( pe.message() );
	// 	traceUI->alert( msg );
	// 	return false;
	// } catch( TextureMapException e ) {
	// 	string msg( "Texture mapping exception: " );
	// 	msg.append( e.message() );
	// 	traceUI->alert( msg );
	// 	return false;
	// }

	// if (!sceneLoaded())
	// 	return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	size_t newBufferSize = w * h * 3;
	if (newBufferSize != buffer.size()) {
		bufferSize = newBufferSize;
		buffer.resize(bufferSize);
	}
	buffer_width = w;
	buffer_height = h;
	std::fill(buffer.begin(), buffer.end(), 0);
	m_bBufferReady = true;

	/*
	 * Sync with TraceUI
	 */

	threads = traceUI->getThreads();
	block_size = traceUI->getBlockSize();
	thresh = traceUI->getThreshold();
	samples = traceUI->getSuperSamples();
	aaThresh = traceUI->getAaThreshold();

	// YOUR CODE HERE
	// FIXME: Additional initializations
	
	glm::dvec3 center = {0.0, water::sea_level+1.0, water::water_size/2.0};
	glm::dvec3 direction = glm::normalize(-1.0*center);
	for(int x = 0; x < photon_resolution; x++){
		for(int z = 0; z < photon_resolution; z++){
			double jitterx = glm::linearRand<double>(water::water_size/-photon_resolution,water::water_size/photon_resolution);
			double jitterz = glm::linearRand<double>(water::water_size/-photon_resolution,water::water_size/photon_resolution);
			glm::dvec3 loc = glm::vec3(center.x - water::water_size/2.0 + x*(water::water_size/(photon_resolution-1)) + jitterx, center.y, 
										center.z - water::water_size/2.0 + z*(water::water_size/(photon_resolution-1)) + jitterz);
			Photon p(loc, direction, 1.0f);
			simulate_photon(p, 1, true);
		}
	}
	std::cout << "---------------------------------------------------------" << std::endl;
	std::cout << "SCATTERING: " << std::endl;
	std::cout << "---------------------------------------------------------" << std::endl;
	ss_map.update_kd();
	// hack_ss_buffer.resize(buffer_width*buffer_height);
	// std::fill(hack_ss_buffer.begin(), hack_ss_buffer.end(), nullptr);
	std::cout << "---------------------------------------------------------" << std::endl;
	std::cout << "CAUSTICS: " << std::endl;
	std::cout << "---------------------------------------------------------" << std::endl;
	caustics_map.update_kd();
}

void RayTracer::simulate_photon(Photon& p, int depth, bool flag){

	if (depth < 0 || p.strength < 0.0001)
		return;

	double start_height = p.location.y - water::sea_level;

	if (start_height > 0 && p.direction.y < 0) { // approx. sun photon
		glm::dvec3 path = glm::dvec3(p.direction) * -1.0*(start_height/p.direction.y);
		glm::dvec3 waterhit_coords = glm::dvec3(p.location) + path;

		glm::dvec3 normal = ws.normal(glm::vec2(waterhit_coords.x, waterhit_coords.z));

		/* ABSORB: 0.1,   REFRACT: 0.9 */
		double n_i = 1.0;
		double n_t = water::water_index;
		double cosine = glm::dot(normal, p.direction);
		double root = 1.0 - (n_i/n_t)*(n_i/n_t)*(1.0 - cosine*cosine);
		if (root >= 0){ // not total internal reflection
			glm::vec3 dir = glm::normalize(
					((n_i/n_t)*cosine - sqrt(root))*normal
					-(n_i/n_t)*p.direction);
			Photon p2(waterhit_coords, dir, p.strength*0.9);
			//std::cout << glm::to_string(p.direction) << ", " << glm::to_string(path) << std::endl;
			simulate_photon(p2, depth-1, flag);
		}
		else { //TIR
			Photon p2(waterhit_coords, glm::normalize(glm::reflect<3, double>(p.direction, normal)), p.strength*0.1);
			simulate_photon(p2, depth-1, false);
		}
		
	}
	else if (start_height > 0 && p.direction.y > 0) { // approx. exited photon
		return;
	}
	else if (start_height <= 0 && p.direction.y > 0 && will_exit(p.direction, p.location)) { // approx. exiting photon
		return; // maybe add scatter
	}
	else { // approx. scattering/caustic photon
		double step_size = 0.01;
		// std::cout << glm::to_string(p.location) << std::endl;
		while(p.location.x > water::water_size/-2.0 && p.location.x < water::water_size/2.0 && p.location.z > water::water_size/-2.0 && p.location.z < water::water_size/2.0){
			if (p.location.y > water::seafloor_level){ // scatter
				double dep = (p.location.y-water::seafloor_level)/(water::sea_level-water::seafloor_level);
				double scat = glm::linearRand<double>(0,1);
				if (scat < pow(dep, 2.0)){
					double r = glm::linearRand<double>(0,1);
					Photon p2(p.location + (step_size*r)*p.direction, glm::vec3(0), p.strength*0.01*r);
					ss_map.add_photon(p2);
					// glm::dvec3 ray = ((p.location + (step_size*2.0*(r-0.5))*p.direction) - scene->getCamera().getEye();

					simulate_photon(p2, depth-1, false);
				}
			}
			else if (flag){ // caustic
				Photon p2(glm::vec3(p.location.x, water::seafloor_level, p.location.z), glm::vec3(0), p.strength*0.7);
				caustics_map.add_photon(p2);
			}
			p.location += step_size*p.direction;
			//std::cout << glm::to_string(p.location) << std::endl;
		}
	}
}

bool RayTracer::will_exit(glm::dvec3 direction, glm::dvec3 location){
	if (direction.y > 0) {
		double x_exit = ((water::sea_level-location.y)/direction.y)*direction.x;
		double z_exit = ((water::sea_level-location.y)/direction.y)*direction.z;
		return (x_exit < water::water_size/-2.0 || x_exit > water::water_size/2.0 
				|| z_exit < water::water_size/-2.0 || z_exit < water::water_size/2.0);
	}
	else {
		double x_exit = ((water::seafloor_level-location.y)/direction.y)*direction.x;
		double z_exit = ((water::seafloor_level-location.y)/direction.y)*direction.z;
		return (x_exit < water::water_size/-2.0 || x_exit > water::water_size/2.0 
				|| z_exit < water::water_size/-2.0 || z_exit < water::water_size/2.0);
	}

	
}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
	// Always call traceSetup before rendering anything.
	traceSetup(w,h);

	// YOUR CODE HERE
	// FIXME: Start one or more threads for ray tracing
	//
	// TIPS: Ideally, the traceImage should be executed asynchronously,
	//       i.e. returns IMMEDIATELY after working threads are launched.
	//
	//       An asynchronous traceImage lets the GUI update your results
	//       while rendering.

	for(int i = 0; i < w; i++){
		for(int j = 0; j < h; j++){
			tracePixel(i, j);
		}
	}
}

int RayTracer::aaImage()
{
	// YOUR CODE HERE
	// FIXME: Implement Anti-aliasing here
	//
	// TIP: samples and aaThresh have been synchronized with TraceUI by
	//      RayTracer::traceSetup() function
	return 0;
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	//        This is a helper routine for GUI.
	//
	// TIPS: Introduce an array to track the status of each worker thread.
	//       This array is maintained by the worker threads.
	return true;
}

void RayTracer::waitRender()
{
	// YOUR CODE HERE
	// FIXME: Wait until the rendering process is done.
	//        This function is essential if you are using an asynchronous
	//        traceImage implementation.
	//
	// TIPS: Join all worker threads here.
}


glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

