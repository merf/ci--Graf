#pragma once
#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include "cinder/Xml.h"
#include "cinder/TriMesh.h"
#include "types.h"

#include "TagPoint.h"

//---------------------------------------------------------------------------------------------------------
struct GrafDrawingParams
{
	static float g_RotationAmount;
	static float g_ZExtrusion;	
	static float g_MaxSpeed;	
	static float g_MinSpeed;	
	static float g_WidthMulY;	
	static float g_WidthMulX;
	static float g_BrushSize;
	static float g_SplineSubdivs;
	static float g_CircleSubdivs;
	static float g_UpdateSpeed;
};

class CSimulation;
//---------------------------------------------------------------------------------------------------------
class CTagStroke
{
public:
	CTagStroke(CSimulation* p_phys, const ci::XmlTree& stroke_xml);
	~CTagStroke();

	typedef std::vector <CTagPoint*> TPointList;

	const TPointList&					GetData() const { return m_Points; }
	void								ComputeTangents();
	void								ComputePTF();
	void								Normalise();
	void								ComputeWidths();
	void								AttachSprings();

	void								Reset();
	void								Clear() { m_Points.clear(); }

	void								Update(float update_time);

	bool								HasActiveTransitions();
	void								ResetTransition(ETransitionType type);

private:
	TPointList							m_Points;
};

//---------------------------------------------------------------------------------------------------------
class CTag
{
public:
	CTag(std::string file_path);
	~CTag();
	
	void								Update();
	void								Draw();
	
	void								Reset();

	const std::string&					GetArtist() const { return m_Artist; }

	ETransitionType						GetCurrTransition() const { return m_CurrTransition; }
	void								ResetTransition(ETransitionType type);
	
private:
	void								ParseXML(ci::XmlTree& xml);
	
	int									GetNumStrokes() const { return m_Strokes.size(); }	
	const CTagStroke::TPointList&	GetStrokeData(int i) const { return m_Strokes[i]->GetData(); }

	void								DrawSegment(ci::TriMesh& tri_mesh, const ci::Vec3f& p1, float w1, ci::Quatf& q1, const ci::Vec3f& p2, float w2, ci::Quatf& q2, u32& curr_index);

	//TODO - could pass in a profile to draw along the curve rather than assuming a circle.
	void								AddSegmentVertices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Vec4f& colour, ci::Quatf& orientation, u32 subdivs, u32& curr_index);
	void								AddSegmentVerticesAndIndices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Vec4f& colour, ci::Quatf& orientation, u32 subdivs, u32& curr_index);

	std::string							m_Artist;
	typedef std::vector<CTagStroke*> TStrokeList;
	TStrokeList							m_Strokes;

	ETransitionType						m_CurrTransition;


	CSimulation*						mp_Phys;
};
