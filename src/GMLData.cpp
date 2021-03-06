#include "cinder/app/App.h"

#include <limits>
#include "GMLData.h"
#include "cinder/Xml.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/Quaternion.h"

#include "Spline.h"

#include "Physics/Simulation.h"


using namespace ci;
using namespace std;

float GrafDrawingParams::g_RotationAmount	= 0.4f;
float GrafDrawingParams::g_ZExtrusion		= 100.0f;
float GrafDrawingParams::g_MaxSpeed			= 20;
float GrafDrawingParams::g_MinSpeed			= 0.1f;
float GrafDrawingParams::g_BrushSize		= 0.1f;
float GrafDrawingParams::g_CircleSubdivs	= 10;
float GrafDrawingParams::g_SplineSubdivs	= 5;
float GrafDrawingParams::g_UpdateSpeed		= 0.05f;


//*******************************************************************************************************
//*******************************************************************************************************
CTagStroke::CTagStroke(CSimulation* p_phys, const XmlTree& stroke_xml)
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

		CTagPoint* stroke_data_point = new CTagPoint(p_phys, x, y, -z, t);
		stroke_data_point->Reset();
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
			m_Points.push_back(new CTagPoint(p_phys, p1.x, p1.y, (float)i, (float)i));
			width *= 1.1f;
		}
	}
}

//*******************************************************************************************************
CTagStroke::~CTagStroke()
{
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		delete *it;
	}
}

//*******************************************************************************************************
void CTagStroke::Reset()
{
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		(*it)->Reset();
	}
}

//*******************************************************************************************************
void CTagStroke::ComputeWidths()
{
	bool first = true;
	Vec3f prev_point;
	float prev_time = 0;
	float max_width = 0.0f;
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if(!first)
		{
			//TODO - this is a rubbish way to calculate speed since corners will give large
			//values, better to use the spline.
			float time = fabsf((*it)->GetTime() - prev_time);
			Vec3f dir = (*it)->GetPos() - prev_point;
			(*it)->SetDesiredWidth(dir.length() / time);
			(*it)->SetDesiredWidth(dir.length());
			if((*it)->GetDesiredWidth() > max_width)
			{
				max_width = (*it)->GetDesiredWidth();
			}
		}
		else
		{
			(*it)->SetDesiredWidth(0.0f);
		}

		prev_point = (*it)->GetPos();
		prev_time = (*it)->GetTime();

		first = false;
	}
	
	m_Points[m_Points.size()-1]->SetDesiredWidth(0);

	if(max_width > 0)
	{
		float one_on_max_width = 1.0f / max_width;
		for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		{
			float w = (*it)->GetDesiredWidth() * one_on_max_width;
			w = pow(w,2);
			(*it)->SetDesiredWidth(w);
		}
	}
}

//*******************************************************************************************************
void CTagStroke::AttachSprings()
{
	//if(m_Points.size() > 2)
	//{
	//	TPointList::iterator it1 = m_Points.begin();
	//	TPointList::iterator it2 = it1+1;
	//	TPointList::iterator it3 = it2+1;

	TPointList::iterator it1 = m_Points.begin();
	TPointList::iterator it2 = it1;
	++it2;
	TPointList::iterator it3 = it2;
	++it3;
	
	for(; it3 != m_Points.end();)
	{
		(*it1)->SetupPhysicsObjects(*it2, *it3);
		++it1;
		++it2;
		++it3;
	}
	
	--it2;
	--it3;
	
	(*it2)->SetupPhysicsObjects(*it3, NULL);
	(*it3)->SetupPhysicsObjects(NULL, NULL);
	
	
	//}
}

////*******************************************************************************************************
//void CGMLDataStroke::ComputeTangents()
//{
//	u32 i0 = 0;
//	u32 i1 = 0;
//	u32 i2 = 1;
//	u32 i3 = 2;
//
//	for(u32 i=0; i<m_Points.size()-2; ++i)
//	{
//		Vec3f p1, p2;
//		
//		Spline::CatmullRom(p1, 0.0f, m_Points[i0].GetPos(), m_Points[i1].GetPos(), m_Points[i2].GetPos(), m_Points[i3].GetPos());
//		Spline::CatmullRom(p2, 0.001f, m_Points[i0].GetPos(), m_Points[i1].GetPos(), m_Points[i2].GetPos(), m_Points[i3].GetPos());
//
//		Vec3f t = p2 - p1;
//		t.normalize();
//		m_Points[i].SetTangent(t);
//
//		i0 = i1;
//		i1 = i2;
//		i2 = i3;
//		++i3;
//	}
//}

//*******************************************************************************************************
void CTagStroke::Normalise()
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
		Vec3f p = (*it)->GetDesiredPos();
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

	if(z_size == 0)
	{
		z_size = 1;
	}
	
	float one_on_x_size = 2.0f / x_size;
	float one_on_y_size = 2.0f / y_size;
	float one_on_z_size = 1.0f / z_size;

	Vec3f origin(min_x, min_y, min_z);
	Vec3f mul(one_on_x_size, one_on_y_size, one_on_z_size);
	//Vec3f centre(-0.5f, -0.5f, -0.5f);
	Vec3f centre(-1.0f, -1.0f, -0.5f);

	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		Vec3f pos = (*it)->GetDesiredPos();
		pos -= origin;
		pos *= mul;
		pos += centre;
		(*it)->SetPos(pos);
	}
}

//*******************************************************************************************************
void CTagStroke::Update(float update_time)
{
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		(*it)->Update(update_time);
	}
}

//*******************************************************************************************************
bool CTagStroke::HasActiveTransitions()
{
	bool active_transitions = false;
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if((*it)->HasActiveTransitions())
		{
			active_transitions = true;
		}
	}

	return active_transitions;
}

//*******************************************************************************************************
void CTagStroke::ResetTransition(ETransitionType type)
{
	for(TPointList::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if(it != m_Points.begin())
		{
			(*it)->SetUpTransitioners(type);
		}
	}
}

////*******************************************************************************************************
//void CGMLDataStroke::ComputePTF()
//{
//	//      m[0] = Imath::firstFrame( p[0], p[1], p[2] );
//	//      for( int i = 1; i < n - 1; i++ )
//	//      {
//	//          m[i] = Imath::nextFrame( m[i-1], p[i-1], p[i], t[i-1], t[i] );
//	//      }
//	//      m[n-1] = Imath::lastFrame( m[n-2], p[n-2], p[n-1] );
//
//	int n = m_Points.size();
//	// Make sure we have at least 3 points because the first frame requires it
//	if( n >= 3 ) 
//	{
//		// Make the parallel transport frame
//		m_Points[0].m_Frame = firstFrame(m_Points[0].GetPos(), m_Points[1].GetPos(),  m_Points[2].GetPos());
//		
//		// Make the remaining frames - saving the last
//		for(int i = 1; i < n - 1; ++i ) 
//		{
//			Vec3f prevT = m_Points[i-1].m_Tangent;
//			Vec3f curT  = m_Points[i].m_Tangent;
//			m_Points[i].m_Frame = nextFrame(m_Points[i - 1].m_Frame, m_Points[i - 1].GetPos(), m_Points[i].GetPos(), prevT, curT);
//
//			Vec3f p = m_Points[i].m_Frame.getTranslation();
//			std::shared_ptr<cinder::msw::dostream> mOutputStream = shared_ptr<cinder::msw::dostream>( new cinder::msw::dostream );
//			*mOutputStream << p.x << ", " << p.y << "\n";
//		}
//		// Make the last frame
//		m_Points[n - 1].m_Frame = lastFrame(m_Points[n - 2].m_Frame, m_Points[n - 2].GetPos(), m_Points[n - 1].GetPos());
//	}
//}

//*******************************************************************************************************
//*******************************************************************************************************
CTag::CTag(std::string file_path)
:
m_CurrTransition(TRANSITION_IN)
{
	static float time_step = 1.0f/30.0f;
	mp_Phys = new CSimulation(time_step);
	mp_Phys->AddGlobalForce(new CGravity(ci::Vec3f(0, 0, 9.81f)));

	XmlTree doc(loadFile(file_path));
	ParseXML(doc);

	for(TStrokeList::iterator it = m_Strokes.begin(); it != m_Strokes.end(); ++it)
	{
		//TODO - need to perform our normalisation pass across all strokes so that they remain in the same space.
		(*it)->Normalise();
		(*it)->ComputeWidths();
		//it->ComputeTangents();
		//it->ComputePTF();
		(*it)->AttachSprings();
	}
}

//*******************************************************************************************************
CTag::~CTag()
{
	for(TStrokeList::iterator it = m_Strokes.begin(); it != m_Strokes.end(); ++it)
	{
		delete *it;
	}

	//delete mp_Phys;
}

//*******************************************************************************************************
void CTag::Reset()
{
	for(TStrokeList::iterator it = m_Strokes.begin(); it != m_Strokes.end(); ++it)
	{
		(*it)->Reset();
	}

	m_CurrTransition = TRANSITION_IN;
}

//*******************************************************************************************************
void CTag::ParseXML(XmlTree& xml_tree)
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
					const XmlTree& client = item->getChild("client");
					const XmlTree& name = client.getChild("name");
					m_Artist = name.getValue();
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
						CTagStroke* stroke = new CTagStroke(mp_Phys, *stroke_it);
						m_Strokes.push_back(stroke);
					}
				}
			}
		}
	}	
}

//*******************************************************************************************************
void CTag::ResetTransition(ETransitionType type)
{
	m_CurrTransition = type;

	for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
	{
		m_Strokes[i_stroke]->ResetTransition(m_CurrTransition);
	}

}

//*******************************************************************************************************
void CTag::Update()
{
	static float time_step = 1.0f/30.0f;
	mp_Phys->Update(time_step);
	
	for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
	{
		float update_time = GrafDrawingParams::g_UpdateSpeed;
		if(m_CurrTransition == TRANSITION_OUT)
		{
			update_time *= 5;
		}

		m_Strokes[i_stroke]->Update(update_time);
	}

	switch(m_CurrTransition)
	{
	case TRANSITION_IN:
		{
			bool active_transitions = false;

			//check for all transitions finished.
			for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
			{
				if(m_Strokes[i_stroke]->HasActiveTransitions())
				{
					active_transitions = true;
				}
			}

			if(!active_transitions)
			{
				ResetTransition(TRANSITION_WAIT);
			}
		}
		break;
	case TRANSITION_OUT:
		{
			bool active_transitions = false;

			//check for all transitions finished.
			for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
			{
				if(m_Strokes[i_stroke]->HasActiveTransitions())
				{
					active_transitions = true;
				}
			}

			if(!active_transitions)
			{
				ResetTransition(TRANSITION_END);
			}
		}
		break;
	default:
		break;
	}
}

//*******************************************************************************************************
void CTag::Draw()
{
	if(true)
	{
		//mp_Phys->Draw();
		//return;
	}

	ci::TriMesh tri_mesh;

	for(u32 i_stroke=0; i_stroke<m_Strokes.size(); ++i_stroke)
	{
		const CTagStroke::TPointList& points = m_Strokes[i_stroke]->GetData();

		u32 curr_index = 0;

		u32 p1 = 0;
		u32 p2 = 0;
		u32 p3 = 1;
		u32 p4 = 2;

		u32 num = points.size();
		for(u32 i_point=0; i_point<num-1; ++i_point)
		{
			//retrieve control points.
			Vec3f v1 = points[p1]->GetPos();
			Vec3f v2 = points[p2]->GetPos();
			Vec3f v3 = points[p3]->GetPos();
			Vec3f v4 = points[p4]->GetPos();

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
				
				Spline::CatmullRom(start_width, t0, points[p1]->GetWidth(), points[p2]->GetWidth(), points[p3]->GetWidth(), points[p4]->GetWidth());
				Spline::CatmullRom(end_width, t1, points[p1]->GetWidth(), points[p2]->GetWidth(), points[p3]->GetWidth(), points[p4]->GetWidth());

				start_width *= GrafDrawingParams::g_BrushSize;
				end_width *= GrafDrawingParams::g_BrushSize;

				Vec4f colour = lerp(points[p2]->GetColour(), points[p3]->GetColour(), t1);

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
					AddSegmentVertices(tri_mesh, start, start_width, colour, ori, (u32)GrafDrawingParams::g_CircleSubdivs, curr_index);
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
					AddSegmentVerticesAndIndices(tri_mesh, end, end_width, colour, ori, (u32)GrafDrawingParams::g_CircleSubdivs, curr_index);
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

			if(!points[p3]->IsActive())
			{
				//break;
			}
		}
	}

	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//glColor3f(1,1,1);

	gl::enableWireframe();
	//gl::enableAlphaBlending();
	//gl::enableAdditiveBlending();
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//gl::disableDepthRead();
	//gl::disableDepthWrite();

	gl::draw(tri_mesh);

	gl::disableWireframe();
	gl::disableAlphaBlending();
}

//*******************************************************************************************************
void CTag::AddSegmentVertices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Vec4f& colour, ci::Quatf& orientation, u32 subdivs, u32& curr_index)
{
	float angle_inc = (float)M_PI * 2.0f / (float) subdivs;

	for(u32 i=0; i<subdivs; ++i)
	{
		float angle = i*angle_inc;
		Vec3f offset = Vec3f(0, cos(angle), -sin(angle));
		Vec3f final_point = point + orientation * offset * width;

		tri_mesh.appendVertex(final_point);
		float f = i / (float)subdivs;

		Vec3f normal = final_point - point;
		normal.normalize();

		tri_mesh.appendNormal(normal);

		//gl::drawLine(final_point, point);
		
		ci::Vec4f new_colour = colour;

		//new_colour = Vec4f(1,0,0,0);
		//new_colour = ci::Vec4f(1,0,0,0) + colour;
		//if(i%4 == 0 || i%4==1)
		//	new_colour = ci::Vec4f(0,0,1,0) + colour;

		//new_colour = ci::Vec4f(1,1,1,1);

		//colour = Vec4f(1,0,0,1);

		//glMaterialfv( GL_BACK, GL_DIFFUSE,	colour);

		tri_mesh.appendColorRGBA(ColorA(new_colour.x, new_colour.y, new_colour.z, new_colour.w));
		//tri_mesh.appendColorRGB(Color(f, 0, 1-f));
	}
}

//*******************************************************************************************************
void CTag::AddSegmentVerticesAndIndices(ci::TriMesh& tri_mesh, const ci::Vec3f& point, float width, ci::Vec4f& colour, ci::Quatf& orientation, u32 subdivs, u32& curr_index)
{
	AddSegmentVertices(tri_mesh, point, width, colour, orientation, subdivs, curr_index);

	//do indices.
	for(u32 i=0; i<subdivs; ++i)
	{
		tri_mesh.appendTriangle(curr_index+i,	curr_index+i+subdivs,			curr_index+i+1);
		tri_mesh.appendTriangle(curr_index+i+1,	curr_index+i+subdivs,			curr_index+i+subdivs+1);
	}

	curr_index += subdivs;
}