#pragma once
#include <vector>

#include "GMLData.h"


//*************************************************************************************************************************
//*************************************************************************************************************************
class CTagCollection
{
public:
	CTagCollection();
	~CTagCollection();

	void					NextTag();
	void					ResetCurrent();
	void					FadeOut();
	void					ReadTagData();
	
	void					Update();
	void					Draw();
	const CTag&				GetCurrTag();
	
private:
	unsigned int			m_CurrTag;
	std::vector<CTag*>	m_TagData;	
};