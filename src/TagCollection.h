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
	
	void					Update();
	void					Draw();
	const CTag&			GetCurrTag();
	
private:
	unsigned int			m_CurrTag;
	std::vector<CTag>	m_TagData;	
};