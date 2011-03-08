/*
 *  ForceGenerator.cpp
 *  RPhysics
 *
 *  Created by Neil Wallace on 26/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ForceGenerator.h"
#include "cinder/gl/gl.h"

using namespace ci;

//*********************************************************************************
void CSpring::ApplyForce()
{
	Vec3f direction = m_SimObjectA->GetCurrPos() - m_SimObjectB->GetCurrPos();
	float curr_length = direction.length();
	if(curr_length > 0)
	{
		direction.normalize();
		Vec3f force = -m_Stiffness * ((curr_length - m_RestLength) * direction);
		
		//add damping force
		force += -m_Damping * (m_SimObjectA->GetVelocity() - m_SimObjectB->GetVelocity()).dot(direction) * direction;
		
		m_SimObjectA->ApplyForce(force);
		m_SimObjectB->ApplyForce(-force);
	}
}

//*********************************************************************************
void CSpring::Draw()
{
	glColor3f(0,0,1);
	gl::drawLine(m_SimObjectA->GetCurrPos(), m_SimObjectB->GetCurrPos());
}

//*********************************************************************************
//*********************************************************************************
void CTensionSpring::ApplyForce()
{
	Vec3f direction = m_SimObjectA->GetCurrPos() - m_SimObjectB->GetCurrPos();
	float curr_length = direction.length();
	if(curr_length > m_RestLength)
	{
		direction.normalize();
		Vec3f force = -m_Stiffness * ((curr_length - m_RestLength) * direction);
		
		//add damping force
		force += -m_Damping * (m_SimObjectA->GetVelocity() - m_SimObjectB->GetVelocity()).dot(direction) * direction;
		
		m_SimObjectA->ApplyForce(force);
		m_SimObjectB->ApplyForce(-force);
	}
}

