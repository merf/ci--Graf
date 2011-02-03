#pragma once
#include "types.h"
#include <list>

//We want to be able to specify different ways that points can come into/outof existence
//inflating/deflating, bouncing, moving into place, drifting away
//We want a nice method for achieving this where we can just plug in these transitioner
//components and they update the points properties.

class CTagPoint;

//---------------------------------------------------------------------------------------------------------
class CTagPointTransitionerBase
{
public:
	virtual void Update() = 0;
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
	CBouncer(T* p_current, T* p_desired, T initial_velocity, float tension, float damping) :
	CTemplateTransitionerBase(p_current, p_desired),
		m_Velocity(initial_velocity),
		m_Tension(tension),
		m_Damping(damping)
	{
	}

	virtual void Update();

protected:
	T		m_Velocity;

	float	m_Tension;
	float	m_Damping;
};

//---------------------------------------------------------------------------------------------------------
template <class T>
class CLerper : public CTemplateTransitionerBase<T>
{
public:
	CLerper(T* p_current, T* p_desired, float lerp_speed) :
	CTemplateTransitionerBase(p_current, p_desired),
		m_LerpSpeed(lerp_speed)
	{
	}

	virtual void Update();

protected:
	float	m_LerpSpeed;
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

	float			GetTime() const { return m_Time; }

	bool			IsActive() const { return m_Timer > m_Time; }

	void			Update();
	void			Reset();

private:
	ci::Vec3f		m_CurrPos;
	ci::Vec3f		m_DesiredPos;

	float			m_Time;
	float			m_DesiredWidth;
	float			m_CurrWidth;

	std::list<CTagPointTransitionerBase*>	m_Transitioners;

	float			m_Timer;
};