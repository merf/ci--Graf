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
float GrafDrawingParams::g_CircleSubdivs	= 8;
float GrafDrawingParams::g_SplineSubdivs	= 4;


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
	int i0 = 0;
	int i1 = 0;
	int i2 = 1;
	int i3 = 2;
	for(int i=0; i<m_Points.size()-2; ++i)
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
	tri_mesh.clear();
	
	int curr_index = 0;

	//Vec3f scale_mul(800, 600, GrafDrawingParams::g_ZExtrusion);

	for(int i=0; i<GetNumStrokes(); ++i)
	{
		const CGMLDataStroke::TPointList& points = GetStrokeData(i);
		
		CGMLDataStroke::TPointList::const_iterator point_it = points.begin();

		CGMLDataStroke::TPointList::const_iterator p1 = point_it;
		CGMLDataStroke::TPointList::const_iterator p2 = point_it;
		++point_it;
		CGMLDataStroke::TPointList::const_iterator p3 = point_it;
		++point_it;
		CGMLDataStroke::TPointList::const_iterator p4 = point_it;

		int num = points.size();

		for(int i=0; i<num-2; ++i)
		{
			float age = time - p3->m_Time;
			age *= 10.0f;
			float age_mul = max(0.0f, min(age, 1.0f));
			
			Vec3f v0(0,0,0);
			Vec3f v1(0.02f, 0, 0);
			Vec3f v2(0, 0.02f, 0);
			Vec3f v3(0, 0, 0.02f);
			//v0 = p2->m_Frame * v0;
			//v1 = p2->m_Frame * v1;
			//v2 = p2->m_Frame * v2;
			//v3 = p2->m_Frame * v3;
			
			//v0 *= scale_mul;
			//v1 *= scale_mul;
			//v2 *= scale_mul;
			//v3 *= scale_mul;

			Quatf new_ori(p3->m_Frame);
			new_ori.normalize();
			Quatf old_ori(p2->m_Frame);
			old_ori.normalize();

			v0 = p3->m_Pos;
			v1 = new_ori*Vec3f(0.02f, 0, 0) + p3->m_Pos;
			v2 = new_ori*Vec3f(0, 0.02f, 0) + p3->m_Pos;
			v3 = new_ori*Vec3f(0, 0, 0.02f) + p3->m_Pos;

			//glColor3f(1, 0, 0);
			//gl::drawLine(v0, v1);
			//glColor3f(0, 1, 0);
			//gl::drawLine(v0, v2);
			//glColor3f(0, 0, 1);
			//gl::drawLine(v0, v3);

			Vec3f pos = p2->m_Pos;
			gl::drawCube(pos, Vec3f(1,1,1)*0.01f);
			glColor3f(0, 1, 0);
			gl::drawLine(pos, pos+p2->m_Tangent*0.05f);

			glColor3f(0, 0, 0);

			int num_spline_subdivs = (int)GrafDrawingParams::g_SplineSubdivs;
			for(int i=0; i<num_spline_subdivs; ++i)
			{
				float t0 = i/(float)num_spline_subdivs;
				float t1 = (i+1)/(float)num_spline_subdivs;
				
				Vec3f start, end;
				Spline::CatmullRom(start, t0, p1->m_Pos, p2->m_Pos, p3->m_Pos, p4->m_Pos);
				Spline::CatmullRom(end, t1, p1->m_Pos, p2->m_Pos, p3->m_Pos, p4->m_Pos);
				
				//start *= scale_mul;
				//end *= scale_mul;
				
				float start_width = t0 * p3->m_Speed + (1.0f-t0) * p4->m_Speed;
				float end_width = t1 * p3->m_Speed + (1.0f-t1) * p4->m_Speed;
				
				Spline::CatmullRom(start_width, t0, p1->m_Speed, p2->m_Speed, p3->m_Speed, p4->m_Speed);
				Spline::CatmullRom(end_width, t1, p1->m_Speed, p2->m_Speed, p3->m_Speed, p4->m_Speed);
				
				start_width *= GrafDrawingParams::g_BrushSize;
				end_width *= GrafDrawingParams::g_BrushSize;
				
				start_width *= age_mul;
				end_width *= age_mul;

				Quatf start_ori = old_ori.slerpShortestUnenforced(t0, new_ori);
				Quatf end_ori = old_ori.slerpShortestUnenforced(t1, new_ori);
				
				DrawSegment(tri_mesh, start, start_width, start_ori, end, end_width, end_ori, curr_index);
			}

			p1 = p2;
			p2 = p3;
			p3 = p4;
			p4 = ++point_it;
		}
	}
	
	gl::draw(tri_mesh);
}

//*******************************************************************************************************
void CGMLData::DrawSegment(TriMesh& tri_mesh, const Vec3f& p1, float w1, Quatf& q1, const Vec3f& p2, float w2, Quatf& q2, int& curr_index)
{
	int num_subdivs = (int)GrafDrawingParams::g_CircleSubdivs;
	float angle_inc = (float)M_PI * 2.0f / (float) num_subdivs;

	gl::drawLine(p1, p2);

	for(int i=0; i<num_subdivs; ++i)
	{
		float angle_1 = i*angle_inc;
		float angle_2 = (i+1)*angle_inc;

		Vec3f offset_1 = q1 * Vec3f(0, cos(angle_1), -sin(angle_1));
		Vec3f offset_2 = q2 * Vec3f(0, cos(angle_2), -sin(angle_2));
		
		Vec3f v1 = p1 + offset_1 * w1;
		Vec3f v2 = p2 + offset_1 * w2;
		Vec3f v3 = p1 + offset_2 * w1;
		Vec3f v4 = p2 + offset_2 * w2;

		gl::drawLine(v1, v3);
		
		tri_mesh.appendVertex(v1);
		tri_mesh.appendVertex(v2);
		tri_mesh.appendVertex(v3);
		tri_mesh.appendVertex(v4);
		
		//doh! much easier to just use dir from centre to vert fo normals!
		Vec3f n1 = (v1 - p1);
		Vec3f n2 = (v2 - p2);
		Vec3f n3 = (v3 - p1);
		Vec3f n4 = (v4 - p2);
		
		n1.normalize();
		n2.normalize();
		n3.normalize();
		n4.normalize();
		
		/*
		Vec3f e1 = v2 - v1;
		Vec3f e2 = v4 - v2;
		Vec3f e3 = v3 - v4;
		Vec3f e4 = v1 - v3;
		e1.normalize();
		e2.normalize();
		e3.normalize();
		e4.normalize();
		Vec3f n1 = (e1).cross(-e4);
		Vec3f n2 = (e2).cross(-e1);
		Vec3f n3 = (e4).cross(-e3);
		Vec3f n4 = (e3).cross(-e2);
		 */
		
		tri_mesh.appendNormal(n1);
		tri_mesh.appendNormal(n2);
		tri_mesh.appendNormal(n3);
		tri_mesh.appendNormal(n4);
		
		bool clockwise = true;
		if(clockwise)
		{
			tri_mesh.appendTriangle(0 + curr_index, 1 + curr_index, 2 + curr_index);
			tri_mesh.appendTriangle(2 + curr_index, 1 + curr_index, 3 + curr_index);
		}
		else 
		{
			tri_mesh.appendTriangle(0 + curr_index, 2 + curr_index, 1 + curr_index);
			tri_mesh.appendTriangle(3 + curr_index, 1 + curr_index, 2 + curr_index);
		}
		
		curr_index += 4;
	}
}

