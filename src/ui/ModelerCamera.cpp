#ifdef _WIN32
#include <windows.h>
#endif

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

#include <FL/gl.h>
#include <FL/glu.h>

// We include these files from modeler so that we can
// display the rendered image in OpenGL -- for debugging
// purposes.

#include "ModelerCamera.h"

#pragma warning(push)
#pragma warning(disable : 4244)

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502f
#endif 

const float kMouseRotationSensitivity		= 1.0f/90.0f;
const float kMouseTranslationXSensitivity	= 0.03f;
const float kMouseTranslationYSensitivity	= 0.03f;
const float kMouseZoomSensitivity			= 0.08f;


inline glm::vec3 operator * (const glm::mat4x4& mat, const glm::vec3& vec)
{
	glm::vec4 vec4(vec[0], vec[1], vec[2], 1.0);
	auto ret = mat * vec4;
	return glm::vec3(ret[0], ret[1], ret[2]);
}

void ModelerCamera::calculateViewingTransformParameters() 
{
	glm::mat4 dollyXform;
	glm::mat4 azimXform;
	glm::mat4 elevXform;
	glm::mat4 twistXform;
	glm::mat4 originXform;

	dollyXform = glm::translate(glm::vec3(0,0,mDolly));
	azimXform = glm::rotate(mAzimuth, glm::vec3(1, 0, 0));
	elevXform = glm::rotate(mElevation, glm::vec3(0, 1, 0));
	twistXform = glm::mat4x4(1.0f);
	originXform = glm::translate(mLookAt);
	
	mPosition = glm::vec3(0,0,0);
	// grouped for (mat4 * vec3) ops instead of (mat4 * mat4) ops
	mPosition = originXform * (azimXform * (elevXform * (dollyXform * mPosition)));

	if ( fmod((float)mElevation, 2.0f*M_PI) < 3*M_PI/2 && fmod((float)mElevation, 2.0f*M_PI) > M_PI/2 )
		mUpVector= glm::vec3(0,-1,0);
	else
		mUpVector= glm::vec3(0,1,0);

	mDirtyTransform = false;
}

ModelerCamera::ModelerCamera() 
{
	reset();
}

void ModelerCamera::reset()
{
	mElevation = mAzimuth = mTwist = 0.0f;
	mDolly = -20.0f;
	mElevation = 0.2f;
	mAzimuth = (float)M_PI;

	mLookAt = glm::vec3( 0, 0, 0 );
	mCurrentMouseAction = kActionNone;

	calculateViewingTransformParameters();
}

void ModelerCamera::clickMouse( MouseAction_t action, int x, int y )
{
	mCurrentMouseAction = action;
	mLastMousePosition[0] = x;
	mLastMousePosition[1] = y;
}

void ModelerCamera::dragMouse( int x, int y )
{
	glm::vec3 mouseDelta   = glm::vec3(x,y,0.0f) - mLastMousePosition;
	mLastMousePosition = glm::vec3(x,y,0.0f);

	switch(mCurrentMouseAction)
	{
	case kActionTranslate:
		{
			calculateViewingTransformParameters();

			float xTrack =  -mouseDelta[0] * kMouseTranslationXSensitivity;
			float yTrack =  mouseDelta[1] * kMouseTranslationYSensitivity;

			glm::vec3 transXAxis = glm::cross(mUpVector, (mPosition - mLookAt));
			transXAxis = glm::normalize(transXAxis);
			glm::vec3 transYAxis = glm::cross((mPosition - mLookAt), transXAxis);
			transYAxis = glm::normalize(transYAxis);

			setLookAt(getLookAt() + transXAxis * xTrack + transYAxis*yTrack);
			
			break;
		}
	case kActionRotate:
		{
			float dAzimuth		=   -mouseDelta[1] * kMouseRotationSensitivity;
			float dElevation	=   mouseDelta[0] * kMouseRotationSensitivity;
			
			setAzimuth(getAzimuth() + dAzimuth);
			setElevation(getElevation() + dElevation);
			
			break;
		}
	case kActionZoom:
		{
			float dDolly = -mouseDelta[1] * kMouseZoomSensitivity;
			setDolly(getDolly() + dDolly);
			break;
		}
	case kActionTwist:
		// Not implemented
	default:
		break;
	}

}

void ModelerCamera::releaseMouse( int x, int y )
{
	mCurrentMouseAction = kActionNone;
}


void ModelerCamera::applyViewingTransform() {
	if( mDirtyTransform )
		calculateViewingTransformParameters();

	// Place the ModelerCamera at mPosition, aim the ModelerCamera at
	// mLookAt, and twist the ModelerCamera such that mUpVector is up
	gluLookAt(	mPosition[0], mPosition[1], mPosition[2],
				mLookAt[0],   mLookAt[1],   mLookAt[2],
				mUpVector[0], mUpVector[1], mUpVector[2]);

    // Depending on which class you're in, you may have to 
    // implement this yourself!
}

#pragma warning(pop)
