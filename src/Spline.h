#pragma once
#include "cinder/Vector.h"

class Spline
{
public:
	static void CatmullRom(ci::Vec3f& result, float t, const ci::Vec3f& p0, const ci::Vec3f& p1, const ci::Vec3f& p2, const ci::Vec3f& p3)
	{
		//http://www.mvps.org/directx/articles/catmull/
		//q(t) = 0.5 *(    (2 * P1) + 
		//		 (-P0 + P2) * t +
		//		 (2*P0 - 5*P1 + 4*P2 - P3) * t^2 +
		//		 (-P0 + 3*P1- 3*P2 + P3) * t^3) 
		result = 0.5f * ((2 * p1) +
			(p2 - p0) * t +
			(2*p0 - 5*p1 + 4*p2 - p3) * pow(t, 2) +
			(3*p1 - p0 - 3*p2 + p3) * pow(t, 3));
	}

	static void CatmullRom(float& result, float t, const float& p0, const float& p1, const float& p2, const float& p3)
	{
		//http://www.mvps.org/directx/articles/catmull/
		//q(t) = 0.5 *(    (2 * P1) + 
		//		 (-P0 + P2) * t +
		//		 (2*P0 - 5*P1 + 4*P2 - P3) * t^2 +
		//		 (-P0 + 3*P1- 3*P2 + P3) * t^3) 
		result = 0.5f * ((2 * p1) +
			(p2 - p0) * t +
			(2*p0 - 5*p1 + 4*p2 - p3) * pow(t, 2) +
			(3*p1 - p0 - 3*p2 + p3) * pow(t, 3));
	}
};