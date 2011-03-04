/*
 *  Integrator.h
 *  RPhysics
 *
 *  Created by Neil Wallace on 26/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once;
#include "SimObject.h"

class CIntegratorBase
{
public:
	CIntegratorBase(float time_step)
	:
	m_FixedTimeStep(time_step)
	{
	}
	
	virtual void	Integrate(Vec3fArg acceleration, CSimObjectBase& sim_object) = 0;
	
protected:
	float	m_FixedTimeStep;
};

class CForwardEulerIntegrator : public CIntegratorBase
{
public:
	CForwardEulerIntegrator(float time_step) : CIntegratorBase(time_step) {}
	
	virtual void	Integrate(Vec3fArg acceleration, CSimObjectBase& sim_object)
	{
		//calculate new position using the velocity at current time
		sim_object.SetCurrPos(sim_object.GetCurrPos() + sim_object.GetVelocity() * m_FixedTimeStep);
		//calculate new velocity using the acceleration at current time
		sim_object.SetVelocity(sim_object.GetVelocity() + acceleration * m_FixedTimeStep);
	}
	
private:
};

class CVerletNoVelocityIntegrator : public CIntegratorBase
{
public:
	CVerletNoVelocityIntegrator(float time_step)
		: 
	CIntegratorBase(time_step),
	m_Drag(0.005f)
	{

	}

	CVerletNoVelocityIntegrator(float time_step, float drag)
		: 
	CIntegratorBase(time_step),
	m_Drag(drag)
	{

	}

	virtual void Integrate(Vec3fArg acceleration, CSimObjectBase& sim_object)
	{
		ci::Vec3f new_pos = (2 - m_Drag) * sim_object.GetCurrPos()
			- (1 - m_Drag) * sim_object.GetPrevPos()
			+ acceleration * m_FixedTimeStep * m_FixedTimeStep;

		sim_object.SetPrevPos(sim_object.GetCurrPos());
		sim_object.SetCurrPos(new_pos);
	}
private:
	float	m_Drag;
};