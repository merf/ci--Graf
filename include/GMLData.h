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
};

//---------------------------------------------------------------------------------------------------------
class CTagStroke
{
public:
	CTagStroke(const ci::XmlTree& stroke_xml);

	typedef std::vector <CTagPoint> TPointList;

	const TPointList&					GetData() const { return m_Points; }
	void								ComputeWidths();
	void								ComputeTangents();
	void								ComputePTF();
	void								Normalise();

	void								Reset();
	void								Clear() { m_Points.clear(); }

	void								Update();

private:
	TPointList							m_Points;
};

//---------------------------------------------------------------------------------------------------------
class CTag
{
public:
	CTag(std::string file_path);
	
	void								Update();
	void								Draw();
	
	void								Reset();
	
private:
	void								ParseXML(ci::XmlTree& xml);
	
	int									GetNumStrokes() const { return m_Strokes.size(); }	
	const CTagStroke::TPointList&	GetStrokeData(int i) const { return m_Strokes[i].GetData(); }

	void								DrawSegment(ci::TriMesh& tri_mesh, const ci::Vec3f& p1, float w1, ci::Quatf& q1, const ci::Vec3f& p2, float w2, ci::Quatf& q2, u32& curr_index);

	//TODO - could pass in a profile to draw along the curve rather than assuming a circle.
	void								AddSegmentVertices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index);
	void								AddSegmentVerticesAndIndices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index);

	std::string							m_Artist;
	typedef std::vector<CTagStroke> TStrokeList;
	TStrokeList							m_Strokes;
};
