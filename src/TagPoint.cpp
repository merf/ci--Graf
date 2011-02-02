#include "TagPoint.h"

//*******************************************************************************************************
//*******************************************************************************************************
CTagPoint::CTagPoint(float x, float y, float z, float t)
: 
m_Pos(x, y, z), 
m_Time(t), 
m_DesiredWidth(0), 
m_CurrWidth(0), 
m_WidthVel(0), 
m_Timer(0) 
{ 
	Reset();
}

//*******************************************************************************************************
void CTagPoint::Reset()
{
	m_Timer = 0;
	m_CurrWidth = 0;
	m_WidthVel = 0;
}

//*******************************************************************************************************
void CTagPoint::Update()
{
	m_Timer += 0.05f;
	if(IsActive())
	{
		static float vel_mul = 0.5f;
		m_WidthVel += (m_DesiredWidth - m_CurrWidth) * vel_mul;

		m_CurrWidth += m_WidthVel;

		static float damping = 0.9f;
		m_WidthVel *= damping;
	}
}

