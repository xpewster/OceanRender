#include "TraceUI.h"
#include <sys/types.h>
#if defined(_MSC_VER)
#include "../win32/dirent.h"
#else
#include <dirent.h>
#endif
#include "../scene/cubeMap.h"
#include "../scene/material.h"

/*
 * JSON for Modern C++
 * version 3.0.1
 * https://github.com/nlohmann/json
 */
#include "json.hpp"
using Json = nlohmann::json;
#include <fstream>
#include <iostream>

namespace {
template <typename T>
void load(Json& j, const string& field, T& target)
{
	// Do not change its value by default
	target = j.value(field, target);
}

} // anonymous namespace

TraceUI::TraceUI()
{
	for (unsigned int i = 0; i < MAX_THREADS; i++)
		rayCount[i] = 0;
}

TraceUI::~TraceUI()
{
}

void TraceUI::setCubeMap(CubeMap* cm)
{
	cubemap.reset(cm);
}

void TraceUI::loadFromJson(const char* file)
{
	std::ifstream fin(file);
	Json json;
	fin >> json;

	load(json, "threads", m_threads);
	load(json, "size", m_nSize);
	load(json, "recursion_depth", m_nDepth);
	load(json, "threshold", m_nThreshold);
	load(json, "blocksize", m_nBlockSize);
	load(json, "supersamples", m_nSuperSamples);
	load(json, "aa_threshold", m_nAaThreshold);
	load(json, "tree_depth", m_nTreeDepth);
	load(json, "leaf_size", m_nLeafSize);
	load(json, "filter_width", m_nFilterWidth);
	load(json, "anti_alias", m_antiAlias);
	load(json, "kdtree", m_kdTree);
	load(json, "shadows", m_shadows);
	load(json, "smoothshade", m_smoothshade);
	load(json, "backface_culling", m_backface);
	/*
	 * Note for Students:
	 * The following options are legacy from previous semesters.
	 *
	 * THE DEFAULT VALUE (DEFINED IN TraceUI.h) IS THE EXPECTED BEHAVIOUR.
	 * DO NOT CHANGE THEM IN YOUR ASSIGNMENT.
	 */
	load(json, "internal_reflection", m_internalReflection);
	load(json, "backface_specular", m_backfaceSpecular);
}

namespace {
std::vector<string> image_exts = {
	".bmp",
	".png"
};

const char *matcher[][2] = {
	{"pos", "x"},
	{"neg", "x"},
	{"pos", "y"},
	{"neg", "y"},
	{"pos", "z"},
	{"neg", "z"},
};
}

bool TraceUI::matchCubemapFiles(const string& one_cubemap_file,
				string matched_fn[6],
				string& pdir)
{
	DIR *dp;
	struct dirent *ep;
	std::string fN = one_cubemap_file;
	pdir = fN.substr(0, fN.find_last_of("/"));
	dp = opendir(pdir.data());

	if (dp == NULL) {
		std::cerr << "Couldn't open the directory " << pdir << std::endl;
		return false;
	}
	for (int i = 0; i < 6; i++)
		matched_fn[i].clear();
	int matched = 0;
	while ((ep = readdir(dp))) {
		std::string fn(ep->d_name);
		/*
		 * Skip files with unrecognized extensions
		 */
		bool ext_matched = false;
		for (const auto& ext : image_exts) {
			if (fn.find(ext) != string::npos) {
				std::cerr << fn << " matches " << ext  << std::endl;
				ext_matched = true;
				break;
			}
		}
		if (!ext_matched)
			continue;
		for (int i = 0; i < 6; i++) {
			auto pos0 = fn.find(matcher[i][0]);
			if (pos0 == std::string::npos)
				continue;
			auto pos1 =  fn.find(matcher[i][1], pos0);
			if (pos1 == std::string::npos)
				continue;
			if (!matched_fn[i].empty()) {
				(void)closedir(dp);
				std::cerr << matcher[i][0] << matcher[i][1]
					  << " matches " << matched_fn[i]
					  << " and " << fn
					  << ", stop smartload to avoid confliction"
					  << std::endl;
				return false;
			}
			matched_fn[i] = fn;
			matched++;
			break;
		}
		if (matched == 6)
			break;
	}
	(void)closedir(dp);
	if (matched != 6) {
		std::cerr << "Cannot locate all six cubemap files"
			  << std::endl;
		return false;
	}
	return true;
}

void TraceUI::smartLoadCubemap(const string& file)
{
	string matched_fn[6];
	string pdir;
	bool matched = matchCubemapFiles(file, matched_fn, pdir);
	if (matched) {
		if (!getCubeMap()) {
			setCubeMap(new CubeMap());
		}
		try {
			for (int i = 0; i < 6; i++)
				cubemap->setNthMap(i, new TextureMap(pdir + "/" + matched_fn[i]));
		} catch (TextureMapException &xcpt) {
			cubemap.reset();
			std::cerr << xcpt.message() << std::endl;
			return ;
		}
		useCubeMap(true);
	}
}
