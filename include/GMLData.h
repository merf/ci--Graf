#pragma once
#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include "cinder/Xml.h"
#include "cinder/TriMesh.h"
#include <list>

typedef unsigned int u32;

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

//We want to be able to specify different ways that points can come into/outof existence
//inflating/deflating, bouncing, moving into place, drifting away
//We want a nice mmethod for achieving this where we can just plug in these transitioner
//components and they update the points properties.
class CWidthUpdaterComponent
{
public:
//	CWidthUpdaterComponent(CGMLDataPoint* p_parent) {}
};

//---------------------------------------------------------------------------------------------------------
class CGMLDataPoint
{
public:
	CGMLDataPoint(float x, float y, float z, float t) : m_Pos(x, y, z), m_Time(t), m_Speed(0) {}
	ci::Vec3f		m_Pos;
	ci::Vec3f		m_Tangent;
	float			m_Time;
	float			m_Speed;
	
	ci::Matrix44f	m_Frame;
};

//---------------------------------------------------------------------------------------------------------
class CGMLDataStroke
{
public:
	CGMLDataStroke(const ci::XmlTree& stroke_xml);

	typedef std::vector <CGMLDataPoint> TPointList;

	const TPointList&					GetData() const { return m_Points; }
	void								ComputeSpeeds();
	void								ComputeTangents();
	void								ComputePTF();
	void								Normalise();

	void								Clear() { m_Points.clear(); }

private:
	TPointList							m_Points;
};

//---------------------------------------------------------------------------------------------------------
class CGMLData
{
public:
	CGMLData(std::string file_path);
	
	void								Draw(float time);
	
	
private:
	void								ParseXML(ci::XmlTree& xml);
	
	int									GetNumStrokes() const { return m_Strokes.size(); }	
	const CGMLDataStroke::TPointList&	GetStrokeData(int i) const { return m_Strokes[i].GetData(); }

	void								DrawSegment(ci::TriMesh& tri_mesh, const ci::Vec3f& p1, float w1, ci::Quatf& q1, const ci::Vec3f& p2, float w2, ci::Quatf& q2, u32& curr_index);

	//TODO - could pass in a profile to draw along the curve rather than assuming a circle.
	void								AddSegmentVertices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index);
	void								AddSegmentVerticesAndIndices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index);

	std::string							m_Artist;
	typedef std::vector<CGMLDataStroke> TStrokeList;
	TStrokeList							m_Strokes;
};
