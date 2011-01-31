#pragma once
#include <vector>

#include "GMLData.h"


//*************************************************************************************************************************
//*************************************************************************************************************************
class CTagCollection
{
public:
	CTagCollection();	

	void					Reset();
	void					ReadTagData();
	
	void					Update(float update_time) { m_Timer += update_time; }
	void					Draw();
	const CGMLData&			GetCurrTag();
	float					GetTimer() { return m_Timer; }
	
private:
	float					m_Timer;
	unsigned int			m_CurrTag;
	std::vector<CGMLData>	m_TagData;	
};