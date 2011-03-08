/*
 *  Simulation.h
 *  RPhysics
 *
 *  Created by Neil Wallace on 26/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "Integrator.h"
#include "SimObject.h"
#include "ForceGenerator.h"

#include "boost/shared_ptr.hpp"
#include <list>

class CSimulation
{
public:
	CSimulation(float time_step)
	{
		CIntegratorBase* p_integrator = new CVerletNoVelocityIntegrator(time_step);
		m_Integrator = boost::shared_ptr<CIntegratorBase>(p_integrator);
	}
	
	void AddSpring(TSpringPtr& spring) { m_Springs.push_back(spring); }
	void AddGlobalForce(CForceGeneratorBase* p_force) { m_GlobalForces.push_back(TForceGeneratorPtr(p_force)); }
	void AddSimObject(TSimObjectPtr& sim_object) { m_SimObjects.push_back(sim_object); }
	void RemoveSimObject(TSimObjectPtr& sim_object);
	
	void Update(float delta_time);
	void Draw();
	
protected:	
	typedef std::list<TSimObjectPtr > TSimObjectList;
	typedef std::list<TForceGeneratorPtr > TForceGeneratorList;
	
	TSimObjectList			m_SimObjects;
	
	TForceGeneratorList		m_GlobalForces;
	TForceGeneratorList		m_Springs;
	
	boost::shared_ptr<CIntegratorBase>			m_Integrator;
};