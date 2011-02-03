#include "TagPoint.h"

#include <math.h>

using namespace ci;

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> void CBouncer<T>::Update()
{
	m_Velocity += (*CBouncer<T>::mp_Desired - *CBouncer<T>::mp_Current) * m_Tension;

	m_Velocity *= m_Damping;

	*CBouncer<T>::mp_Current += m_Velocity;
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> void CLerper<T>::Update()
{
	*CLerper<T>::mp_Current = lerp(*CLerper<T>::mp_Current, *CLerper<T>::mp_Desired, m_LerpSpeed);
}

//*******************************************************************************************************
//*******************************************************************************************************
CTagPoint::CTagPoint(float x, float y, float z, float t)
: 
m_DesiredPos(x, y, z),
m_CurrPos(x, y, z),
m_Time(t), 
m_DesiredWidth(0),
m_Timer(0) 
{ 
	//Reset();
}

//*******************************************************************************************************
CTagPoint::~CTagPoint()
{
	for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end(); ++it)
	{
		delete *it;
	}
}

//*******************************************************************************************************
void CTagPoint::Reset()
{
	m_Timer = 0;

	for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end(); ++it)
	{
		delete *it;
	}
	m_Transitioners.clear();

	m_CurrWidth = 0;
	CTagPointTransitionerBase* p_trans = new CBouncer<float>(&m_CurrWidth, &m_DesiredWidth, 0.0f, 0.1f, 0.9f);
	m_Transitioners.push_back(p_trans);

	m_CurrPos.set(m_DesiredPos * Vec3f(1, 0, 0) + Vec3f(3, 0, 10));
	p_trans = new CBouncer<Vec3f>(&m_CurrPos, &m_DesiredPos, Vec3f::zero(), 0.05f, 0.85f);
	//p_trans = new CLerper<Vec3f>(&m_CurrPos, &m_DesiredPos, 0.1f);
	m_Transitioners.push_back(p_trans);
}

//*******************************************************************************************************
void CTagPoint::SetDesiredWidth(float width) 
{ 
	m_DesiredWidth = width;
}

//*******************************************************************************************************
void CTagPoint::Update()
{
	m_Timer += 0.05f;
	if(IsActive())
	{
		for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end(); ++it)
		{
			(*it)->Update();
		}
	}
}

//*******************************************************************************************************
void CTagPoint::SetPos(Vec3fArg p)
{ 
	m_DesiredPos = p; 
}

