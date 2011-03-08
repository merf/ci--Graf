/*
 *  ForceGenerator.h
 *  RPhysics
 *
 *  Created by Neil Wallace on 26/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once;
#include "SimObject.h"
#include "boost/shared_ptr.hpp"
#include "cinder/App/App.h"
//-----------------------------------------------------------------------------------------------------
class CForceGeneratorBase
{
public:
	virtual void ApplyGlobalForce(CSimObjectBase& sim_object) = 0;
	virtual void ApplyForce() = 0;

	virtual void Draw() {}
};

typedef boost::shared_ptr<CForceGeneratorBase> TForceGeneratorPtr;

//-----------------------------------------------------------------------------------------------------
class CGravity : public CForceGeneratorBase
{
public:
	CGravity()
	:
	m_Acceleration(0.0f, 9.81f, 0.0f)
	{
	}
	
	CGravity(Vec3fArg acceleration)
	:
	m_Acceleration(acceleration)
	{
	}

	virtual void ApplyGlobalForce(CSimObjectBase& sim_object) { sim_object.ApplyForce(sim_object.GetMass() * m_Acceleration); }
	virtual void ApplyForce() {}

private:
	ci::Vec3f	m_Acceleration;
};

//-----------------------------------------------------------------------------------------------------
class CMedium : public CForceGeneratorBase
{
public:
	CMedium(float drag)
	:
	m_DragCoefficient(drag)
	{
	}
	
	virtual void ApplyGlobalForce(CSimObjectBase& sim_object) { sim_object.ApplyForce(sim_object.GetVelocity() * -m_DragCoefficient); }
	virtual void ApplyForce() {}

private:
	float	m_DragCoefficient;
};

//-----------------------------------------------------------------------------------------------------
class CSpringBase : public CForceGeneratorBase
{
};

//-----------------------------------------------------------------------------------------------------
class CSpring : public CSpringBase
{
public:
	CSpring(float stiffness, float damping, TSimObjectPtr& obj_a, TSimObjectPtr& obj_b)
	:
	m_Stiffness(stiffness),
	m_Damping(damping)
	{
		m_SimObjectA = obj_a;
		m_SimObjectB = obj_b;
		m_RestLength = (obj_a->GetCurrPos() - obj_b->GetCurrPos()).length();
	}
	
	CSpring(float stiffness, float damping, TSimObjectPtr& obj_a, TSimObjectPtr& obj_b, float rest_length)
	:
	m_Stiffness(stiffness),
	m_Damping(damping),
	m_RestLength(rest_length)
	{
		m_SimObjectA = obj_a;
		m_SimObjectB = obj_b;
	}
	
	virtual void ApplyGlobalForce(CSimObjectBase& sim_object) {};
	virtual void ApplyForce();

	virtual void Draw();
	
protected:
	float	m_Stiffness;
	float	m_Damping;
	float	m_RestLength;
	
	TSimObjectPtr m_SimObjectA;
	TSimObjectPtr m_SimObjectB;
};

//-----------------------------------------------------------------------------------------------------
class CTensionSpring : public CSpring
{
public:
	CTensionSpring(float stiffness, float damping, TSimObjectPtr& obj_a, TSimObjectPtr& obj_b)
	:
	CSpring(stiffness, damping, obj_a, obj_b)
	{
	}

	CTensionSpring(float stiffness, float damping, TSimObjectPtr& obj_a, TSimObjectPtr& obj_b, float rest_length)
	:
	CSpring(stiffness, damping, obj_a, obj_b, rest_length)
	{
	}
	
	virtual void ApplyForce();
};

typedef boost::shared_ptr<CSpringBase> TSpringPtr;

