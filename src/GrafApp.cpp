#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "cinder/Camera.h"
#include "cinder/Arcball.h"

#include "GMLData.h"
#include "TagCollection.h"

using namespace ci;
using namespace ci::app;
using namespace std;


Vec3f light_pos;

//*************************************************************************************************************************
//*************************************************************************************************************************
class GrafAppApp : public AppBasic 
{
public:
	void setup();
	void mouseDown(MouseEvent event);	
	void mouseDrag(MouseEvent event);
	void resize(ResizeEvent event);
	void update();
	void draw();
	
	Matrix44f				m_Rotation;
	
	CTagCollection			m_TagCollection;
	
	params::InterfaceGl		m_Params;
	CameraPersp				m_Cam;
	Arcball					mArcball;
};

//*************************************************************************************************************************
void GrafAppApp::setup()
{	
	setWindowSize(800, 600);
	m_Cam.lookAt(Vec3f(0, 0, -2), Vec3f::zero(), Vec3f(0, -1, 0));

	// Arcball
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( getWindowCenter() );
	mArcball.setRadius( 150 );
	mArcball.setConstraintAxis(Vec3f(0,1,0));


	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();

	//mCamera.lookAt(Vec3f(0, -getWindowHeight()*0.5f, -70), Vec3f(getWindowWidth()*0.5f, getWindowHeight()*0.5f, 0));
	m_Rotation.setToIdentity();
	
	light_pos = Vec3f(0, 800, 3000);
	
	m_Params = params::InterfaceGl( "Parameters", Vec2i( 200, 200 ) );
	m_Params.addParam("Rotation", &GrafDrawingParams::g_RotationAmount, "min=-7 max=7 step=0.1 keyIncr=r keyDecr=R");
	m_Params.addParam("Z-Extrusion", &GrafDrawingParams::g_ZExtrusion, "min=0.001 max=5 step=0.1 keyIncr=z keyDecr=Z");
	m_Params.addParam("MaxSpeed", &GrafDrawingParams::g_MaxSpeed, "min=2 max=100 step=1 keyIncr=x keyDecr=X");
	m_Params.addParam("MinSpeed", &GrafDrawingParams::g_MinSpeed, "min=1 max=50 step=1 keyIncr=c keyDecr=C");
	m_Params.addParam("BrushSize", &GrafDrawingParams::g_BrushSize, "min=0 max=1 step=.01 keyIncr=w keyDecr=W");

	m_Params.addParam("CirlceSubdivs", &GrafDrawingParams::g_CircleSubdivs, "min=4 max=32 step=2");
	m_Params.addParam("SplineSubdivs", &GrafDrawingParams::g_SplineSubdivs, "min=2 max=32 step=2");

	m_Params.addParam("LightX", &light_pos.x, "min=-1000 max=1000 step=100");
	m_Params.addParam("LightY", &light_pos.y, "min=-1000 max=1000 step=100");
	m_Params.addParam("LightZ", &light_pos.z, "min=-10000 max=10000 step=1000");
	
}

//*************************************************************************************************************************
void GrafAppApp::mouseDown(MouseEvent event)
{
	Vec2i P = event.getPos();
	P.y = getWindowHeight() - P.y;
	mArcball.mouseDown(P);
}

//*************************************************************************************************************************
void GrafAppApp::mouseDrag(MouseEvent event)
{	
	Vec2i P = event.getPos();
	P.y = getWindowHeight() - P.y;
	mArcball.mouseDrag(P);
}

//*************************************************************************************************************************
void GrafAppApp::resize(ResizeEvent event)
{
	m_Cam.setPerspective(60, getWindowAspectRatio(), 1, 1000);
	gl::setMatrices(m_Cam);	
}

//*************************************************************************************************************************
void GrafAppApp::update()
{
	m_TagCollection.Update(1.0f/getFrameRate());
	
	m_Rotation.setToIdentity();
	m_Rotation.rotate(Vec3f(0, 1, 0), GrafDrawingParams::g_RotationAmount);
}

//*************************************************************************************************************************
void GrafAppApp::draw()
{
	//glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix();
	
	Matrix44f trans;
	
	gl::setMatrices(m_Cam);
	gl::rotate(mArcball.getQuat());	
	
	GLfloat light_position[] = { light_pos.x, light_pos.y, light_pos.z, true };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// clear out the window with black
	gl::clear(Color(1,1,1));

	glColor3f(1,1,1);
	glColor3f(0,0,0);
	
	m_TagCollection.Draw();	
	
	glPopMatrix();
	
	
	params::InterfaceGl::draw();
}

CINDER_APP_BASIC( GrafAppApp, RendererGl )
