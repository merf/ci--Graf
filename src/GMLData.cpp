#include "GMLData.h"
#include "cinder/Xml.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/Utilities.h"

#include "Spline.h"

using namespace ci;
using namespace std;

float GrafDrawingParams::g_RotationAmount	= 0;
float GrafDrawingParams::g_ZExtrusion		= 0.5f;
float GrafDrawingParams::g_MaxSpeed			= 20;
float GrafDrawingParams::g_MinSpeed			= 0.1f;
float GrafDrawingParams::g_BrushSize		= 0.1f;
float GrafDrawingParams::g_CircleSubdivs	= 8;
float GrafDrawingParams::g_SplineSubdivs	= 2;


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
			float time = prev_time - it->m_Time;
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
//*******************************************************************************************************
CGMLData::CGMLData(std::string file_path)
{
	XmlTree doc(loadFile(file_path));
	ParseXML(doc);

	for(TStrokeList::iterator it = m_Strokes.begin(); it != m_Strokes.end(); ++it)
	{
		it->ComputeSpeeds();
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

	Vec3f scale_mul(800, 600, GrafDrawingParams::g_ZExtrusion);

	for(int i=0; i<GetNumStrokes(); ++i)
	{
		bool first = true;
		//Vec3f prev_point;
		float prev_w = 0;
		
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
			//if(point_it->m_Time < m_TagCollection.GetTimer())
			{
				//if(!first)
				{
					Vec3f dir = p3->m_Pos - p2->m_Pos;
					dir *= scale_mul;
					float speed = max(GrafDrawingParams::g_MinSpeed, min(GrafDrawingParams::g_MaxSpeed, dir.length()));
					float width = (GrafDrawingParams::g_MaxSpeed - speed)/(GrafDrawingParams::g_MaxSpeed-GrafDrawingParams::g_MinSpeed);
					width += 0.1f;

					//width = p3->m_Speed;
					
					width *= GrafDrawingParams::g_BrushSize;

					float age = time - p3->m_Time;
					age *= 10.0f;
					float age_mul = max(0.0f, min(age, 1.0f));
					width *= age_mul;
					
					int num_spline_subdivs = (int)GrafDrawingParams::g_SplineSubdivs;
					for(int i=0; i<num_spline_subdivs; ++i)
					{
						float t0 = i/(float)num_spline_subdivs;
						float t1 = (i+1)/(float)num_spline_subdivs;

						Vec3f start, end;
						Spline::CatmullRom(start, t0, p1->m_Pos, p2->m_Pos, p3->m_Pos, p4->m_Pos);
						Spline::CatmullRom(end, t1, p1->m_Pos, p2->m_Pos, p3->m_Pos, p4->m_Pos);

						start *= scale_mul;
						end *= scale_mul;

						float start_width = t0 * width + (1.0f-t0) * prev_w;
						float end_width = t1 * width + (1.0f-t1) * prev_w;

						Spline::CatmullRom(start_width, t0, p1->m_Speed, p2->m_Speed, p3->m_Speed, p4->m_Speed);
						Spline::CatmullRom(end_width, t1, p1->m_Speed, p2->m_Speed, p3->m_Speed, p4->m_Speed);

						start_width *= GrafDrawingParams::g_BrushSize;
						end_width *= GrafDrawingParams::g_BrushSize;

						DrawSegment(tri_mesh, start, start_width, end, end_width, curr_index);
					}
					
					prev_w = width;
				}

				first = false;
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
void CGMLData::DrawSegment(TriMesh& tri_mesh, const Vec3f& p1, float w1, const Vec3f& p2, float w2, int& curr_index)
{
	int num_subdivs = (int)GrafDrawingParams::g_CircleSubdivs;
	float angle_inc = (float)M_PI * 2.0f / (float) num_subdivs;

	//gl::drawLine(p1, p2);

	for(int i=0; i<num_subdivs; ++i)
	{
		float angle_1 = i*angle_inc;
		float angle_2 = (i+1)*angle_inc;
		
		//highly suspicious maths but i think it might work - need to make sure that we arent adding verts twice like present tho.
		Vec3f dir = p2 - p1;
		dir.z = 0;
		dir.normalize();
		dir.x = fabsf(dir.x);
		dir.y = fabsf(dir.y);
		
		dir.x = 0;
		dir.y = 1;
		//dir.y = 1 - dir.x;
		
		//Vec3f offset_1(cos(angle_1) * dir.y, cos(angle_1) * dir.x, -sin(angle_1));
		//Vec3f offset_2(cos(angle_2) * dir.y, cos(angle_2) * dir.x, -sin(angle_2));

		Vec3f offset_1(0, cos(angle_1), -sin(angle_1));
		Vec3f offset_2(0, cos(angle_2), -sin(angle_2));
		
		Vec3f v1 = p1 + offset_1 * w1;
		Vec3f v2 = p2 + offset_1 * w2;
		Vec3f v3 = p1 + offset_2 * w1;
		Vec3f v4 = p2 + offset_2 * w2;

		//gl::drawLine(v1, v3);
		
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

