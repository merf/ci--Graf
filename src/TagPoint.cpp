#include "TagPoint.h"

#include <math.h>

using namespace ci;

template<class T> Perlin CPerlin<T>::m_Perlin = Perlin();

float GetLengthSq(float f)
{
	return f*f;
}

template<class T> float GetLengthSq(T& t)
{
	return t.lengthSquared();
}

void GetNoise(Vec3f& noise, Perlin& perlin, Vec3f& v, float time)
{
	float mul1 = 0.5f;
	float mul2 = 0.01f;
	float mul3 = 0.5f;
	float mul4 = 0.5f;

	noise = Vec3f(perlin.noise(v.x * mul1, time * mul2 + v.y * mul3, v.z * mul4), 
				perlin.noise(v.y * mul1, time * mul2 + v.z * mul3, v.x * mul4), 
				perlin.noise(v.y * mul1, time * mul2 + v.x * mul3, v.y * mul4));
	//noise -= Vec3f(0.5f, 0.5f, 0.5f);
	noise *= 0.01f;
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
template<class T> bool CPerlin<T>::Update()
{
	T noise;
	GetNoise(noise, m_Perlin, *CPerlin<T>::mp_Current, m_Time);

	m_Vel += noise;

	*CPerlin<T>::mp_Current += m_Vel * m_Time;

	m_Time += 0.01f;

	return true;
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> bool CMultiplier<T>::Update()
{
	*CMultiplier<T>::mp_Current *= m_Multiplier;

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
	m_CurrColour = Vec4f(0,0,0,1);
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
	//if(IsActive())
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
	if(IsActive())
	{
		return !m_Transitioners.empty();
	}
	return true;
}

//*******************************************************************************************************
void CTagPoint::SetPos(Vec3fArg p)
{ 
	m_DesiredPos = p;
	m_CurrPos = p;
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
			CTagPointTransitionerBase* p_trans = new CBouncer<float>(&m_CurrWidth, &m_DesiredWidth, 0.0f, 0.2f, 0.8f, 0.01f);
			m_Transitioners.push_back(p_trans);

			//m_CurrPos.set(m_DesiredPos * Vec3f(1, 0, 0) + Vec3f(3, 0, 10));
			//p_trans = new CBouncer<Vec3f>(&m_CurrPos, &m_DesiredPos, Vec3f::zero(), 0.05f, 0.85f, 0.01f);
			p_trans = new CLerper<Vec3f>(&m_CurrPos, &m_DesiredPos, 0.2f, 0.01f);
			//m_Transitioners.push_back(p_trans);

			/*m_CurrColour = Vec4f(0,0,0,0);
			p_trans = new CLerper<Vec4f>(&m_CurrColour, &m_DesiredColour, 0.05f, 0.01f);
			m_Transitioners.push_back(p_trans);*/

			break;
		}
	case TRANSITION_OUT:
		{
			m_DesiredColour = Vec4f(0,0,0,0);
			m_Transitioners.push_back(new CLerper<Vec4f>(&m_CurrColour, &m_DesiredColour, 0.05f, 0.01f));
			m_DesiredWidth = 0;
			m_Transitioners.push_back(new CLerper<float>(&m_CurrWidth, &m_DesiredWidth, 0.05f, 0.01f));
			m_Transitioners.push_back(new CPerlin<Vec3f>(&m_CurrPos));
			m_Transitioners.push_back(new CMultiplier<Vec3f>(&m_CurrPos, Vec3f(1.01f, 0.99f, 1.02f)));
		}
		break;
	default:
		break;
	}
}