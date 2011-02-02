#include "cinder/app/App.h"

#include <mmsystem.h>
#include <limits>
#include "GMLData.h"
#include "cinder/Xml.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/Quaternion.h"

#include "Spline.h"

using namespace ci;
using namespace std;

float GrafDrawingParams::g_RotationAmount	= 0.4f;
float GrafDrawingParams::g_ZExtrusion		= 100.0f;
float GrafDrawingParams::g_MaxSpeed			= 20;
float GrafDrawingParams::g_MinSpeed			= 0.1f;
float GrafDrawingParams::g_BrushSize		= 0.1f;
float GrafDrawingParams::g_CircleSubdivs	= 3;
float GrafDrawingParams::g_SplineSubdivs	= 10;


//*******************************************************************************************************
//*******************************************************************************************************
CGMLDataStroke::CGMLDataStroke(const XmlTree& stroke_xml)
{
	for(XmlTree::ConstIter stroke_point = stroke_xml.begin(); stroke_point != stroke_xml.end(); ++stroke_point)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float t = 0.0f;
		
		const std::list<XmlTree>& stroke_point_elements = stroke_point->getChildren();
		for(std::list<XmlTree>::const_iterator stroke_point_elements_it = stroke_point_elements.begin(); stroke_point_elements_it != stroke_point_elements.end(); ++stroke_point_elements_it)
		{
			if(stroke_point_elements_it->getTag() == "x")
			{
				x = stroke_point_elements_it->getValue<float>(0.0f);
			}
			else if(stroke_point_elements_it->getTag() == "y")
			{
				y = stroke_point_elements_it->getValue<float>(0.0f);
			}
			else if(stroke_point_elements_it->getTag() == "z")
			{
				z = stroke_point_elements_it->getValue<float>(0.0f);
			}
			else if(stroke_point_elements_it->getTag() == "time" || stroke_point_elements_it->getTag() == "t")
			{
				t = stroke_point_elements_it->getValue<float>(0.0f);
			}
		}

		CGMLDataPoint stroke_data_point(x, y, z, t);
		m_Points.push_back(stroke_data_point);
	}

	bool replace_with_debug_points = false;
	if(replace_with_debug_points)
	{
		m_Points.clear();

		float width = 1.0f;
		int num_pts = 15;
		float cirlce_res = 5;
		float circle_inc = (float)M_PI * 2.0f / (float)cirlce_res;
		for(int i=0; i<num_pts; ++i)
		{
			Vec3f p1 = Vec3f(cosf(circle_inc*i), -sinf(circle_inc*i), (float)i) * width;
			m_Points.push_back(CGMLDataPoint(p1.x, p1.y, (float)i, (float)i));
			width *= 1.1f;
		}
	}
}

//*******************************************************************************************************
void CGMLDataStroke::ComputeSpeeds()
{
	bool first = true;
	Vec3f prev_point;
	float prev_time = 0;
	float max_speed = 0.0f;
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if(!first)
		{
			float time = fabsf(it->m_Time - prev_time);
			Vec3f dir = it->m_Pos - prev_point;
			it->m_Speed = dir.length() / time;
			if(it->m_Speed > max_speed)
			{
				max_speed = it->m_Speed;
			}
		}
		else
		{
			it->m_Speed = 0.0f;
		}

		prev_point = it->m_Pos;
		prev_time = it->m_Time;

		first = false;
	}
	
	TPointList::reverse_iterator rit = m_Points.rbegin();
	rit->m_Speed = 0;

	if(max_speed > 0)
	{
		float one_on_max_speed = 1.0f / max_speed;
		for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		{
			it->m_Speed *= one_on_max_speed;
		}
	}
}

//*******************************************************************************************************
void CGMLDataStroke::ComputeTangents()
{
	u32 i0 = 0;
	u32 i1 = 0;
	u32 i2 = 1;
	u32 i3 = 2;

	for(u32 i=0; i<m_Points.size()-2; ++i)
	{
		Vec3f p1, p2;
		
		Spline::CatmullRom(p1, 0.0f, m_Points[i0].m_Pos, m_Points[i1].m_Pos, m_Points[i2].m_Pos, m_Points[i3].m_Pos);
		Spline::CatmullRom(p2, 0.001f, m_Points[i0].m_Pos, m_Points[i1].m_Pos, m_Points[i2].m_Pos, m_Points[i3].m_Pos);

		Vec3f t = p2 - p1;
		t.normalize();
		m_Points[i].m_Tangent = t;

		i0 = i1;
		i1 = i2;
		i2 = i3;
		++i3;
	}
}

//*******************************************************************************************************
void CGMLDataStroke::Normalise()
{
	//we want all our points to lie in a unit cube.
	//should we centre around 0,0,0? certainly makes the maths a bit easier generally.
	//we probably also want to preserve the XY aspect ratio.
	const static float ARBITRARILY_LARGE_FLOAT = 1000000.0f;
	float min_x = ARBITRARILY_LARGE_FLOAT;
	float min_y = ARBITRARILY_LARGE_FLOAT;
	float min_z = ARBITRARILY_LARGE_FLOAT;
	float max_x = -ARBITRARILY_LARGE_FLOAT;
	float max_y = -ARBITRARILY_LARGE_FLOAT;
	float max_z = -ARBITRARILY_LARGE_FLOAT;

	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		Vec3f p = it->m_Pos;
		if(p.x > max_x) { max_x = p.x; }
		if(p.x < min_x) { min_x = p.x; }

		if(p.y > max_y) { max_y = p.y; } 
		if(p.y < min_y) { min_y = p.y; }
		
		if(p.z > max_z) { max_z = p.z; } 
		if(p.z < min_z) { min_z = p.z; }
	}

	float x_size = max_x - min_x;
	float y_size = max_y - min_y;
	float z_size = max_z - min_z;

	float one_on_x_size = 1.0f / x_size;
	float one_on_y_size = 1.0f / y_size;
	float one_on_z_size = 1.0f / z_size;

	Vec3f origin(min_x, min_y, min_z);
	Vec3f mul(one_on_x_size, one_on_y_size, one_on_z_size);
	Vec3f centre(-0.5f, -0.5f, -0.5f);

	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		it->m_Pos -= origin;
		it->m_Pos *= mul;
		it->m_Pos += centre;
	}
}

//*******************************************************************************************************
void CGMLDataStroke::ComputePTF()
{
	//      m[0] = Imath::firstFrame( p[0], p[1], p[2] );
	//      for( int i = 1; i < n - 1; i++ )
	//      {
	//          m[i] = Imath::nextFrame( m[i-1], p[i-1], p[i], t[i-1], t[i] );
	//      }
	//      m[n-1] = Imath::lastFrame( m[n-2], p[n-2], p[n-1] );

	int n = m_Points.size();
	// Make sure we have at least 3 points because the first frame requires it
	if( n >= 3 ) 
	{
		// Make the parallel transport frame
		m_Points[0].m_Frame = firstFrame(m_Points[0].m_Pos, m_Points[1].m_Pos,  m_Points[2].m_Pos);
		
		// Make the remaining frames - saving the last
		for(int i = 1; i < n - 1; ++i ) 
		{
			Vec3f prevT = m_Points[i-1].m_Tangent;
			Vec3f curT  = m_Points[i].m_Tangent;
			m_Points[i].m_Frame = nextFrame(m_Points[i - 1].m_Frame, m_Points[i - 1].m_Pos, m_Points[i].m_Pos, prevT, curT);

			Vec3f p = m_Points[i].m_Frame.getTranslation();
			std::shared_ptr<cinder::msw::dostream> mOutputStream = shared_ptr<cinder::msw::dostream>( new cinder::msw::dostream );
			*mOutputStream << p.x << ", " << p.y << "\n";
		}
		// Make the last frame
		m_Points[n - 1].m_Frame = lastFrame(m_Points[n - 2].m_Frame, m_Points[n - 2].m_Pos, m_Points[n - 1].m_Pos);
	}
}

//*******************************************************************************************************
//*******************************************************************************************************
CGMLData::CGMLData(std::string file_path)
{
	XmlTree doc(loadFile(file_path));
	ParseXML(doc);

	for(TStrokeList::iterator it = m_Strokes.begin(); it != m_Strokes.end(); ++it)
	{
		it->Normalise();
		it->ComputeSpeeds();
		it->ComputeTangents();
		it->ComputePTF();
	}
}

//*******************************************************************************************************
void CGMLData::ParseXML(XmlTree& xml_tree)
{
	if(xml_tree.begin()->getTag() == "GML")
	{
		const std::list<XmlTree>& tags = xml_tree.begin()->getChildren();
		
		for(std::list<XmlTree>::const_iterator tag_it = tags.begin(); tag_it != tags.end(); ++tag_it)
		{
			for(XmlTree::ConstIter item = tag_it->begin(); item != tag_it->end(); ++item)
			{
				if(item->getTag() == "header")
				{
					//m_Data.m_Artist = item->Get
					//m_Data.m_Artist
				}
				else if(item->getTag() == "environment")
				{
					
				}
				else if(item->getTag() == "drawing")
				{
					const std::list<XmlTree>& strokes = item->getChildren();
					for(std::list<XmlTree>::const_iterator stroke_it = strokes.begin(); stroke_it != strokes.end(); ++stroke_it)
					{
						CGMLDataStroke stroke_data(*stroke_it);
						
						
						m_Strokes.push_back(stroke_data);
					}
				}
			}
		}
	}	
}

//*******************************************************************************************************
void CGMLData::Draw(float time)
{
	gl::enableWireframe();

	ci::TriMesh tri_mesh;

	for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
	{
		const CGMLDataStroke::TPointList& points = m_Strokes[i_stroke].GetData();

		u32 curr_index = 0;

		u32 p1 = 0;
		u32 p2 = 0;
		u32 p3 = 1;
		u32 p4 = 2;

		u32 num = points.size();
		for(u32 i_point=0; i_point<num-1; ++i_point)
		{
			//retrieve control points.
			Vec3f v1 = points[p1].m_Pos;
			Vec3f v2 = points[p2].m_Pos;
			Vec3f v3 = points[p3].m_Pos;
			Vec3f v4 = points[p4].m_Pos;

			//subdivide spline.
			int num_spline_subdivs = (int)GrafDrawingParams::g_SplineSubdivs;
			for(int i=0; i<num_spline_subdivs; ++i)
			{
				float t0 = i/(float)num_spline_subdivs;
				float t1 = (i+1)/(float)num_spline_subdivs;

				Vec3f start, end;
				Spline::CatmullRom(start, t0, v1, v2, v3, v4);
				Spline::CatmullRom(end, t1, v1, v2, v3, v4);

				float start_width, end_width;
				
				Spline::CatmullRom(start_width, t0, points[p1].m_Speed, points[p2].m_Speed, points[p3].m_Speed, points[p4].m_Speed);
				Spline::CatmullRom(end_width, t1, points[p1].m_Speed, points[p2].m_Speed, points[p3].m_Speed, points[p4].m_Speed);

				start_width *= GrafDrawingParams::g_BrushSize;
				end_width *= GrafDrawingParams::g_BrushSize;

				Vec3f forward = end - start;
				forward.normalize();
				Vec3f right = forward.cross(Vec3f(0,0,1.0f));
				right.normalize();
				Vec3f up = forward.cross(right);
				up.normalize();
				Matrix44f m = Matrix44f(forward, right, up);
				Quatf ori(m);

				//bleugh.
				if(p2 == 0)
				{
					AddSegmentVertices(tri_mesh, start, start_width, ori, (u32)GrafDrawingParams::g_CircleSubdivs, curr_index);
				}
				//if(p3 == num - 1 && i == num_spline_subdivs - 1)
				//{
				//	tri_mesh.appendVertex(end);
				//	u32 subdivs = (u32)GrafDrawingParams::g_CircleSubdivs;
				//	u32 end_index = curr_index + subdivs;

				//	for(u32 i=0; i<subdivs; ++i)
				//	{
				//		tri_mesh.appendTriangle(curr_index+i,	end_index,			curr_index+i+1);
				//	}
				//}
				//else
				{
					AddSegmentVerticesAndIndices(tri_mesh, end, end_width, ori, (u32)GrafDrawingParams::g_CircleSubdivs, curr_index);
				}
			}

			//increment indices.
			p1=p2;
			p2=p3;
			p3=p4;
			++p4;
			if(p4 > num-1)
			{
				p4 = num - 1;
			}
		}
	}

	gl::draw(tri_mesh);
}

//*******************************************************************************************************
void CGMLData::AddSegmentVertices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index)
{
	float angle_inc = (float)M_PI * 2.0f / (float) subdivs;

	for(u32 i=0; i<subdivs; ++i)
	{
		float angle = i*angle_inc;
		Vec3f offset = Vec3f(0, cos(angle), -sin(angle));
		Vec3f final_point = point + orientation * offset * width;

		tri_mesh.appendVertex(final_point);
	}
}

//*******************************************************************************************************
void CGMLData::AddSegmentVerticesAndIndices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Quatf& orientation, u32 subdivs, u32& curr_index)
{
	AddSegmentVertices(tri_mesh, point, width, orientation, subdivs, curr_index);

	//do indices.
	for(u32 i=0; i<subdivs; ++i)
	{
		tri_mesh.appendTriangle(curr_index+i,	curr_index+i+subdivs,			curr_index+i+1);
		tri_mesh.appendTriangle(curr_index+i+1,	curr_index+i+subdivs,			curr_index+i+subdivs+1);
	}

	curr_index += subdivs;
}