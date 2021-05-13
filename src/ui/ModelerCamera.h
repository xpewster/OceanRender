// This ModelerCamera stuff mostly by ehsu

#ifndef ModelerCamera_H
#define ModelerCamera_H

#include <glm/vec3.hpp>

//==========[ class ModelerCamera ]===================================================

typedef enum { kActionNone, kActionTranslate, kActionRotate, kActionZoom, kActionTwist,} MouseAction_t;

class ModelerCamera {
    
protected:
    
    float		mElevation;
    float		mAzimuth;
    float		mDolly;
    float		mTwist; // Not implemented yet
    
    glm::vec3		mLookAt;
    
    glm::vec3		mPosition;
    glm::vec3		mUpVector;
    bool		mDirtyTransform;
    
    void calculateViewingTransformParameters();
    
    glm::vec3			mLastMousePosition;
    MouseAction_t	mCurrentMouseAction;
    
    
public:
    
    //---[ Constructors ]----------------------------------
    
    // defaults to (0,0,0) facing down negative z axis
    ModelerCamera();
	void reset();
    
    //---[ Settings ]--------------------------------------
    
    inline void setElevation( float elevation ) 
    { 
        // don't want elevation to be negative
        if (elevation<0) elevation+=6.28318530717f;
        mElevation = elevation; mDirtyTransform = true; 
    }
    inline float getElevation() const
    { return mElevation; }
    
    inline void setAzimuth( float azimuth )
    { mAzimuth = azimuth; mDirtyTransform = true; }
    inline float getAzimuth() const
    { return mAzimuth; }
    
    inline void setDolly( float dolly )
    { mDolly = dolly; mDirtyTransform = true; }
    inline float getDolly() const
    { return mDolly; }
    
    inline void setTwist( float twist )
    { mTwist = twist; mDirtyTransform = true; }
    inline float getTwist() const
    { return mTwist; }
    
    inline void setLookAt( const glm::vec3 &lookAt )
    { mLookAt = lookAt; mDirtyTransform = true;}
    inline glm::vec3 getLookAt() const
    { return mLookAt; }
    
    //---[ Interactive Adjustment ]------------------------
    // these should be used from a mouse event handling routine that calls
    // the startX method on a mouse down, updateX on mouse move and finally
    // endX on mouse up.
    //-----------------------------------------------------
    void clickMouse( MouseAction_t action, int x, int y );
    void dragMouse( int x, int y );
    void releaseMouse( int x, int y );
    
    //---[ Viewing Transform ]--------------------------------
    void applyViewingTransform();

	// gluLookAt equivalent
	void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up);
};

#endif
