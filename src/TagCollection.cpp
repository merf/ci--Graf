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
: 
m_CurrTag(0) 
{ 
	Reset(); 
	ReadTagData(); 
}
	
//*************************************************************************************************************************
CTagCollection::~CTagCollection()
{
	for(std::vector<CTag*>::iterator it = m_TagData.begin(); it != m_TagData.end(); ++it)
	{
		delete *it;
	}
}

//*************************************************************************************************************************
void CTagCollection::Reset() 
{ 
	m_CurrTag++; 
	if(m_CurrTag >= m_TagData.size()) 
	{ 
		m_CurrTag = 0; 
	}

	if(m_CurrTag < m_TagData.size())
	{
		m_TagData[m_CurrTag]->Reset();
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
				m_TagData.push_back(new CTag(str));
			}
		}
	}
}

//*************************************************************************************************************************
const CTag& CTagCollection::GetCurrTag()
{ 
	return *m_TagData[m_CurrTag]; 
}

//*************************************************************************************************************************
void CTagCollection::Update()
{ 
	m_TagData[m_CurrTag]->Update();
}

//*************************************************************************************************************************
void CTagCollection::Draw()
{ 
	m_TagData[m_CurrTag]->Draw(); 
}