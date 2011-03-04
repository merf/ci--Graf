/*
 *  SimObject.h
 *  RPhysics
 *
 *  Created by Neil Wallace on 26/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once;
#include "cinder/Vector.h"

enum ESimObjectType
{
	SIM_OBJECT_STATIC,
	SIM_OBJECT_DYNAMIC,
};


typedef const ci::Vec3f& Vec3fArg;

//-----------------------------------------------------------------------------------------------------
class CSimObjectBase
{
public:
	CSimObjectBase(ESimObjectType type, float mass)
	:
	m_Mass(mass),
	m_Type(type),
	m_CurrPos(0,0,0),
	m_PrevPos(0,0,0),
	m_Velocity(0,0,0),
	m_ResultantForce(0,0,0)
	{
	}
	
	virtual void	Update(float DeltaTime) = 0;
	
	void			ResetForces() { m_ResultantForce = m_ResultantForce.zero(); }
	
	float			GetMass() const { return m_Mass; }
	
	void			SetCurrPos(Vec3fArg pos) { m_CurrPos = pos; }
	ci::Vec3f&		GetCurrPos() { return m_CurrPos; }
	
	void			SetPrevPos(Vec3fArg pos) { m_PrevPos = pos; }
	ci::Vec3f&		GetPrevPos() { return m_PrevPos; }
	
	void			SetVelocity(Vec3fArg pos) { m_Velocity = pos; }
	ci::Vec3f&		GetVelocity() { return m_Velocity; }

	void			SetResultantForce(Vec3fArg pos) { m_ResultantForce = pos; }
	ci::Vec3f&		GetResultantForce() { return m_ResultantForce; }
	
	ESimObjectType	GetType() const { return m_Type; }
	
	void			ApplyForce(Vec3fArg force) { m_ResultantForce += force; }
	
protected:
	float			m_Mass;
	ESimObjectType	m_Type;
	
	ci::Vec3f		m_CurrPos;
	ci::Vec3f		m_PrevPos;
	ci::Vec3f		m_Velocity;
	
	ci::Vec3f		m_ResultantForce;
};

typedef boost::shared_ptr<CSimObjectBase> TSimObjectPtr;

//-----------------------------------------------------------------------------------------------------
class CSimObject : public CSimObjectBase
{
public:
	CSimObject(ESimObjectType type, float mass, Vec3fArg pos)
	: 
	CSimObjectBase(type, mass)
	{
		m_CurrPos = pos;
		m_PrevPos = pos;
	}
	
	virtual void Update(float time) {}
};

