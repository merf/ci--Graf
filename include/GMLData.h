#pragma once
#include "cinder/Vector.h"
#include "cinder/Xml.h"
#include "cinder/TriMesh.h"
#include <list>


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
class CGMLDataPoint
{
public:
	CGMLDataPoint(float x, float y, float z, float t) : m_Pos(x, y, z), m_Time(t), m_Speed(0) {}
	ci::Vec3f	m_Pos;
	float		m_Time;
	float		m_Speed;
};

//---------------------------------------------------------------------------------------------------------
class CGMLDataStroke
{
public:
	CGMLDataStroke(const ci::XmlTree& stroke_xml);

	const std::list<CGMLDataPoint>&		GetData() const { return m_Points; }
	void								ComputeSpeeds();

	typedef std::list <CGMLDataPoint> TPointList;

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

	void								DrawSegment(ci::TriMesh& tri_mesh, const ci::Vec3f& p1, float w1, const ci::Vec3f& p2, float w2, int& curr_index);

	std::string							m_Artist;
	typedef std::vector<CGMLDataStroke> TStrokeList;
	TStrokeList							m_Strokes;
};
