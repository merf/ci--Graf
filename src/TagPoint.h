#pragma once
#include "types.h"
#include <list>

#include "cinder/Perlin.h"

//We want to be able to specify different ways that points can come into/outof existence
//inflating/deflating, bouncing, moving into place, drifting away
//We want a nice method for achieving this where we can just plug in these transitioner
//components and they update the points properties.

class CTagPoint;

enum ETransitionType
{
	TRANSITION_IN,
	TRANSITION_WAIT,
	TRANSITION_OUT,
	TRANSITION_END,
};

//---------------------------------------------------------------------------------------------------------
class CTagPointTransitionerBase
{
public:
	virtual bool Update() = 0;
	
	virtual bool IsRequiredToFinish() { return true; }
};
//---------------------------------------------------------------------------------------------------------
template <class T>
class CTemplateTransitionerBase : public CTagPointTransitionerBase
{
public:
	CTemplateTransitionerBase(T* p_current, T* p_desired)
	:
	mp_Current(p_current),
	mp_Desired(p_desired)
	{
	}
	
protected:
	T*		mp_Current;
	T*		mp_Desired;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CBouncer : public CTemplateTransitionerBase<T>
{
public:
	CBouncer(T* p_current, T* p_desired, T initial_velocity, float tension, float damping, float allowed_diff) 
	:
	CTemplateTransitionerBase<T>(p_current, p_desired),
	m_Velocity(initial_velocity),
	m_Tension(tension),
	m_Damping(damping),
	m_AllowedDiffSq(allowed_diff*allowed_diff)
	{
	}
	
	virtual bool Update();
	
protected:
	T		m_Velocity;
	
	float	m_Tension;
	float	m_Damping;
	
	float	m_AllowedDiffSq;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CLerper : public CTemplateTransitionerBase<T>
{
public:
	CLerper(T* p_current, T* p_desired, float lerp_speed, float allowed_diff) 
	:
	CTemplateTransitionerBase<T>(p_current, p_desired),
	m_LerpSpeed(lerp_speed),
	m_AllowedDiffSq(allowed_diff*allowed_diff)
	{
	}
	
	virtual bool Update();
	
protected:
	float	m_LerpSpeed;
	float	m_AllowedDiffSq;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CTimeLine : public CTemplateTransitionerBase<T>
{
public:
	CTimeLine(T* p_current, T* p_desired, float time) 
	:
	CTemplateTransitionerBase<T>(p_current, p_desired),
	m_Initial(*p_current),
	m_TotalTime(time),
	m_CurrTime(0)
	{
	}
	
	virtual bool Update();
	
protected:
	T		m_Initial;
	float	m_TotalTime;
	float	m_CurrTime;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CPerlin : public CTemplateTransitionerBase<T>
{
public:
	CPerlin(T* p_current) 
	:
	CTemplateTransitionerBase<T>(p_current, p_current),
	m_Time(0),
	m_Vel(0,0,0)
	{
	}
	
	virtual bool Update();
	virtual bool IsRequiredToFinish() { return false; }
	
	static ci::Perlin m_Perlin;
	
protected:
	float	m_Time;
	T		m_Vel;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CMultiplier : public CTemplateTransitionerBase<T>
{
public:
	CMultiplier(T* p_current, const T& mul) 
	:
	CTemplateTransitionerBase<T>(p_current, p_current),
	m_Multiplier(mul)
	{
	}
	
	virtual bool Update();
	virtual bool IsRequiredToFinish() { return false; }
	
protected:
	T	m_Multiplier;
};

//---------------------------------------------------------------------------------------------------------
class CTagPoint
{
public:
	CTagPoint(float x, float y, float z, float t);
	~CTagPoint();

	Vec3fArg		GetPos() const { return m_CurrPos; }
	Vec3fArg		GetDesiredPos() const { return m_DesiredPos; }
	void			SetPos(Vec3fArg p);

	float			GetWidth() const { return m_CurrWidth; }
	float			GetDesiredWidth() const { return m_DesiredWidth; }
	void			SetDesiredWidth(float width);

	const ci::Vec4f& GetColour() const { return m_CurrColour; }		

	float			GetTime() const { return m_Time; }

	bool			IsActive() const { return m_Timer > m_Time; }

	void			Update();
	void			Reset();
	bool			HasActiveTransitions();

	void			SetUpTransitioners(ETransitionType type);
	void			ClearTransitioners();
private:
	ci::Vec3f		m_CurrPos;
	ci::Vec3f		m_DesiredPos;

	float			m_CurrWidth;
	float			m_DesiredWidth;

	ci::Vec4f		m_CurrColour;
	ci::Vec4f		m_DesiredColour;

	float			m_Time;

	std::list<CTagPointTransitionerBase*>	m_Transitioners;

	float			m_Timer;
};