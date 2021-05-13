#include <cmath>
#include <assert.h>
#include <algorithm>

#include "Box.h"

using namespace std;

const double HUGE_DOUBLE = 1e100;

bool Box::intersectLocal(ray& r, isect& i) const
{
        glm::dvec3 p = r.getPosition();
        glm::dvec3 d = r.getDirection();
//        d.normalize();

        int it;
        double x, y, t, bestT; 
        int mod0, mod1, mod2, bestIndex;

        bestT = HUGE_DOUBLE;
        bestIndex = -1;

        for(it=0; it<6; it++){ 
                mod0 = it%3;

                if(d[mod0] == 0){
                        continue;
                }
                
                t = ((it/3) - 0.5 - p[mod0]) / d[mod0];                 

                if(t < RAY_EPSILON || t > bestT){
                        continue;
                }

                mod1 = (it+1)%3;
                mod2 = (it+2)%3;
                x = p[mod1]+t*d[mod1];
                y = p[mod2]+t*d[mod2];
                
                if(     x<=0.5 && x>=-0.5 &&
                        y<=0.5 && y>=-0.5)
                {
                        if(bestT > t){
                                bestT = t;
                                bestIndex = it;
                        }
                }                       
                
        }

        if(bestIndex < 0) return false;
        
        i.setT(bestT);
        i.setObject(this);
		i.setMaterial(this->getMaterial());

		//glm::dvec3 intersect_point = r.at((float)i.t);
		glm::dvec3 intersect_point = r.at(i);

		int i1 = (bestIndex + 1) % 3;
		int i2 = (bestIndex + 2) % 3;

        if(bestIndex < 3)
		{
                i.setN(glm::dvec3(-double(bestIndex == 0), -double(bestIndex == 1), -double(bestIndex == 2)));
				i.setUVCoordinates( glm::dvec2(	0.5 - intersect_point[ min(i1, i2) ], 
											0.5 + intersect_point[ max(i1, i2) ] ) );
		}
        else
		{
                i.setN(glm::dvec3(double(bestIndex==3), double(bestIndex == 4), double(bestIndex == 5)));
				i.setUVCoordinates( glm::dvec2(	0.5 + intersect_point[ min(i1, i2) ],
											0.5 + intersect_point[ max(i1, i2) ] ) );

		}
        return true;
}
