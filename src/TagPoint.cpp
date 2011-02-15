#include "TagPoint.h"
#include "GMLData.h"
#include "cinder/app/AppBasic.h"

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
	float mul2 = 0.1f;
	float mul3 = 0.15f;
	float mul4 = 0.5f;

	time = ci::app::getElapsedFrames() * mul2;

	noise = Vec3f(perlin.noise((v.x + time) * mul1, time + v.y * mul3, v.z * mul4), 
		perlin.noise((v.y + time) * mul1, time + v.z * mul3, v.x * mul4), 
		perlin.noise((v.y + time) * mul1, time + v.x * mul3, v.y * mul4));
	//noise -= Vec3f(0.5f, 0.5f, 0.5f);
}

void GetNoise(float& noise, Perlin& perlin, Vec3f& v, float time)
{
	float mul1 = 0.5f;
	float mul2 = 0.1f;
	float mul3 = 0.15f;
	float mul4 = 0.5f;

	time = ci::app::getElapsedFrames() * mul2;

	noise = perlin.noise((v.x + time) * mul1, time + v.y * mul3, v.z * mul4);
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
template<class T> bool CTimeLine<T>::Update()
{
	float f = m_CurrTime / m_TotalTime;
	
	*CTimeLine<T>::mp_Current = lerp(CTimeLine<T>::m_Initial, *CTimeLine<T>::mp_Desired, f);
	
	if(f > 1)
	{
		return false;
	}
	
	m_CurrTime += GrafDrawingParams::g_UpdateSpeed;

	return true;
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> bool CPerlin<T>::Update()
{
	T noise;
	GetNoise(noise, m_Perlin, CPerlin<T>::m_Pos, m_Time);

	m_Vel += noise  * m_Scale * GrafDrawingParams::g_UpdateSpeed;

	*CPerlin<T>::mp_Current += m_Vel * m_Time;

	m_Time += 0.01f;

	return true;
}

//*******************************************************************************************************
//*******************************************************************************************************
template<class T> bool CMultiplier<T>::Update()
{
	(*CMultiplier<T>::mp_Current) *= m_Multiplier;

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
void CTagPoint::Update(float upate_time)
{
	m_Timer += upate_time;

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
	//if(IsActive())
	{
		if(m_Transitioners.empty())
		{
			return false;
		}
		else
		{
			bool needs_to_finish = false;
			for(std::list<CTagPointTransitionerBase*>::iterator it = m_Transitioners.begin(); it != m_Transitioners.end(); ++it)
			{
				if((*it)->IsRequiredToFinish())
				{
					needs_to_finish = true;
				}
			}
			
			if(!needs_to_finish)
			{
				return false;
			}
		}
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
	ClearTransitioners();
	
	m_Timer = 0;
	
	switch(type)
	{
	case TRANSITION_IN:
		{
			//m_CurrWidth = m_DesiredWidth * 3;
			//m_Transitioners.push_back(new CBouncer<float>(&m_CurrWidth, &m_DesiredWidth, 0.0f, 0.2f, 0.8f, 0.01f));
			m_Transitioners.push_back(new CTimeLine<float>(&m_CurrWidth, &m_DesiredWidth, 1));

			
			//m_CurrPos.set(m_DesiredPos * Vec3f(1, 0, 0) + Vec3f(3, 0, 10));
			//m_CurrPos.set(m_DesiredPos * Vec3f(1, 0, 1));
			//m_Transitioners.push_back(new CBouncer<Vec3f>(&m_CurrPos, &m_DesiredPos, Vec3f::zero(), 0.03f, 0.85f, 0.01f));
			//m_Transitioners.push_back(new CLerper<Vec3f>(&m_CurrPos, &m_DesiredPos, 0.02f, 0.01f));

			m_CurrColour = Vec4f(0,0,0,0);
			m_Transitioners.push_back(new CTimeLine<Vec4f>(&m_CurrColour, &m_DesiredColour, 1));

			//m_Transitioners.push_back(new CPerlin<Vec3f>(&m_CurrPos, 0.01f, Vec3f(0,0,0), m_CurrPos));

			break;
		}
	case TRANSITION_OUT:
		{
			m_DesiredColour = Vec4f(0,0,0,0);
			m_Transitioners.push_back(new CTimeLine<Vec4f>(&m_CurrColour, &m_DesiredColour, 10));
			m_DesiredWidth *= 2;
			//m_Transitioners.push_back(new CTimeLine<float>(&m_CurrWidth, &m_DesiredWidth, 5));
			m_Transitioners.push_back(new CPerlin<Vec3f>(&m_CurrPos, 0.01f, Vec3f(0,0,0), m_CurrPos));
			m_Transitioners.push_back(new CPerlin<float>(&m_CurrWidth, 0.01f, 0, m_CurrPos));
			m_Transitioners.push_back(new CMultiplier<Vec3f>(&m_CurrPos, Vec3f(1.002f, 0.999f, 1.0f)));
		}
		break;
	default:
		break;
	}
}