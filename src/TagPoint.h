#pragma once
#include "types.h"

//We want to be able to specify different ways that points can come into/outof existence
//inflating/deflating, bouncing, moving into place, drifting away
//We want a nice method for achieving this where we can just plug in these transitioner
//components and they update the points properties.

//---------------------------------------------------------------------------------------------------------
class CTagPoint
{
public:
	CTagPoint(float x, float y, float z, float t);

	Vec3fArg		GetPos() const { return m_Pos; }
	void			SetPos(Vec3fArg p) { m_Pos = p; }

	float			GetWidth() const { return m_CurrWidth; }
	void			SetDesiredWidth(float width) { m_DesiredWidth = width; }

	float			GetTime() const { return m_Time; }

	bool			IsActive() const { return m_Timer > m_Time; }

	void			Update();
	void			Reset();

private:
	ci::Vec3f		m_Pos;
	float			m_Time;

	float			m_DesiredWidth;
	float			m_CurrWidth;
	float			m_WidthVel;
	float			m_Timer;

	//ci::Vec3f		m_Tangent;
	//ci::Matrix44f	m_Frame;
};