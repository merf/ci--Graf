#include "TagCollection.h"
#include "cinder/Xml.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"


#ifdef WIN32
std::string DATA_DIR = "../data/tags/";
#else
std::string DATA_DIR = "../../../data/tags/";
#endif

using namespace ci;
using namespace std;

//*************************************************************************************************************************
//*************************************************************************************************************************
CTagCollection::CTagCollection() 
: m_Timer(0), 
m_CurrTag(0) 
{ 
	Reset(); 
	ReadTagData(); 
}
	
//*************************************************************************************************************************
void CTagCollection::Reset() 
{ 
	m_Timer = 0; 
	m_CurrTag++; 
	if(m_CurrTag >= m_TagData.size()) 
	{ 
		m_CurrTag = 0; 
	} 
}

//*************************************************************************************************************************
void CTagCollection::ReadTagData()
{
	fs::path p(DATA_DIR);
	for( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it ) 
	{
		if( ! is_directory( *it ) )
		{
			if(it->path().extension() == ".xml")
			{
				//console() << "   " << it->path().filename() << " size(bytes): " << fs::file_size( *it ) << std::endl;
				fs::path file = it->path();
				std::string str = file.string();
				m_TagData.push_back(CGMLData(str));
			}
		}
	}
}

//*************************************************************************************************************************
const CGMLData& CTagCollection::GetCurrTag()
{ 
	return m_TagData[m_CurrTag]; 
}

//*************************************************************************************************************************
void CTagCollection::Draw()
{ 
	m_TagData[m_CurrTag].Draw(m_Timer); 
}