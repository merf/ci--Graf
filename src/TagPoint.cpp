#include "TagPoint.h"

#include <math.h>

using namespace ci;

float GetLengthSq(float f)
{
	return f*f;
}

template<class T> float GetLengthSq(T& t)
{
	return t.lengthSquared();
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> bool CBouncer<T>::Update()
{
	T diff = *CBouncer<T>::mp_Desired - *CBouncer<T>::mp_Current;
	m_Velocity += diff * m_Tension;

	m_Velocity *= m_Damping;

	*CBouncer<T>::mp_Current += m_Velocity;

	if(GetLengthSq(diff) < m_AllowedDiffSq && GetLengthSq(m_Velocity) < m_AllowedDiffSq)
	{
		return false;
	}

	return true;
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> bool CLerper<T>::Update()
{
	*CLerper<T>::mp_Current = lerp(*CLerper<T>::mp_Current, *CLerper<T>::mp_Desired, m_LerpSpeed);

	T diff = *CLerper<T>::mp_Current - *CLerper<T>::mp_Desired;
	if(GetLengthSq(diff) < m_AllowedDiffSq)
	{
		return false;
	}

	return true;
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
	m_DesiredColour = Vec4f(0,0,0,1);
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

	ClearTransitioners();
	SetUpTransitioners(TRANSITION_IN);
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
		for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end();)
		{
			if(!(*it)->Update())
			{
				delete *it;
				it = m_Transitioners.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

//*******************************************************************************************************
bool CTagPoint::HasActiveTransitions()
{
	return !m_Transitioners.empty();
}

//*******************************************************************************************************
void CTagPoint::SetPos(Vec3fArg p)
{ 
	m_DesiredPos = p; 
}

//*******************************************************************************************************
void CTagPoint::ClearTransitioners()
{
	for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end(); ++it)
	{
		delete *it;
	}
	m_Transitioners.clear();
}

//*******************************************************************************************************
void CTagPoint::SetUpTransitioners(ETransitionType type)
{
	switch(type)
	{
	case TRANSITION_IN:
		{
			m_CurrWidth = 0;
			CTagPointTransitionerBase* p_trans = new CBouncer<float>(&m_CurrWidth, &m_DesiredWidth, 0.0f, 0.1f, 0.9f, 0.01f);
			m_Transitioners.push_back(p_trans);

			m_CurrPos.set(m_DesiredPos * Vec3f(1, 0, 0) + Vec3f(3, 0, 10));
			p_trans = new CBouncer<Vec3f>(&m_CurrPos, &m_DesiredPos, Vec3f::zero(), 0.05f, 0.85f, 0.01f);
			//p_trans = new CLerper<Vec3f>(&m_CurrPos, &m_DesiredPos, 0.1f);
			m_Transitioners.push_back(p_trans);

			m_CurrColour = Vec4f(0,0,0,0);
			p_trans = new CLerper<Vec4f>(&m_CurrColour, &m_DesiredColour, 0.05f, 0.01f);
			m_Transitioners.push_back(p_trans);

			break;
		}
	case TRANSITION_OUT:
		{
			m_DesiredColour = Vec4f(0,0,0,0);
			m_Transitioners.push_back(new CLerper<Vec4f>(&m_CurrColour, &m_DesiredColour, 0.05f, 0.01f));
		}
		break;
	default:
		break;
	}
}