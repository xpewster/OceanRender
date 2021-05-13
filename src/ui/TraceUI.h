//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __TraceUI_h__
#define __TraceUI_h__

#include <string>
#include <memory>
#define MAX_THREADS 32

using std::string;

class RayTracer;
class CubeMap;

class TraceUI {
public:
	TraceUI();
	virtual ~TraceUI();

	virtual int run() = 0;

	// Send an alert to the user in some manner
	virtual void alert(const string& msg) = 0;

	// setters
	virtual void setRayTracer(RayTracer* r) { raytracer = r; }
	void useCubeMap(bool b) { m_usingCubeMap = b; }

	// accessors:
	int getSize() const { return m_nSize; }
	int getDepth() const { return m_nDepth; }
	int getBlockSize() const { return m_nBlockSize; }
	double getThreshold() const { return (double)m_nThreshold * 0.001; }
	double getAaThreshold() const { return (double)m_nAaThreshold * 0.001; }
	int getSuperSamples() const { return m_nSuperSamples; }
	int getMaxDepth() const { return m_nTreeDepth; }
	int getLeafSize() const { return m_nLeafSize; }
	int getFilterWidth() const { return m_nFilterWidth; }
	int getThreads() const { return m_threads; }
	bool aaSwitch() const { return m_antiAlias; }
	bool kdSwitch() const { return m_kdTree; }
	bool shadowSw() const { return m_shadows; }
	bool smShadSw() const { return m_smoothshade; }
	bool bkFaceSw() const { return m_backface; }
	bool cubeMap() const { return m_usingCubeMap && cubemap; }
	CubeMap* getCubeMap() const { return cubemap.get(); }
	void setCubeMap(CubeMap* cm);
	bool internalReflection() const { return m_internalReflection; }
	bool backfaceSpecular() const { return m_backfaceSpecular; }

	// ray counter
	static void addRays(int number, int ctr)
	{
		if (ctr >= 0)
			rayCount[ctr] += number;
	}
	static void addRay(int ctr)
	{
		if (ctr >= 0)
			rayCount[ctr]++;
	}
	static int getCount(int ctr) { return ctr < 0 ? -1 : rayCount[ctr]; }
	static int getCount()
	{
		int total = 0;
		for (int i = 0; i < m_threads; i++)
			total += rayCount[i];
		return total;
	}
	static int resetCount(int ctr)
	{
		if (ctr < 0)
			return -1;
		int temp = rayCount[ctr];
		rayCount[ctr] = 0;
		return temp;
	}
	static int resetCount()
	{
		int total = 0;
		for (int i = 0; i < m_threads; i++) {
			total += rayCount[i];
			rayCount[i] = 0;
		}
		return total;
	}

	static int m_threads; // number of threads to run
	static bool m_debug;

	static bool matchCubemapFiles(const string& one_cubemap_file,
	                              string matched_fn[6],
	                              string& pdir);
protected:
	RayTracer* raytracer = nullptr;

	int m_nSize = 512;        // Size of the traced image
	int m_nDepth = 0;         // Max depth of recursion
	int m_nThreshold = 0;     // Threshold for interpolation within block
	int m_nBlockSize = 4;     // Blocksize (square, even, power of 2 preferred)
	int m_nSuperSamples = 3;  // Supersampling rate (1-d) for antialiasing
	int m_nAaThreshold = 100; // Pixel neighborhood difference for supersampling
	int m_nTreeDepth = 15;    // maximum kdTree depth
	int m_nLeafSize = 10;     // target number of objects per leaf
	int m_nFilterWidth = 1;   // width of cubemap filter

	static int rayCount[MAX_THREADS]; // Ray counter

	// Determines whether or not to show debugging information
	// for individual rays.  Disabled by default for efficiency
	// reasons.
	bool m_displayDebuggingInfo = false;
	bool m_antiAlias = false;    // Is antialiasing on?
	bool m_kdTree = true;        // use kd-tree?
	bool m_shadows = true;       // compute shadows?
	bool m_smoothshade = true;   // turn on/off smoothshading?
	bool m_backface = true;      // cull backfaces?
	bool m_usingCubeMap = false; // render with cubemap
	bool m_internalReflection = true; // Enable reflection inside a translucent object.
	bool m_backfaceSpecular = false; // Enable specular component even seeing through the back of a translucent object.

	std::unique_ptr<CubeMap> cubemap;

	void loadFromJson(const char* file);
	void smartLoadCubemap(const string& file);
};

#endif
