#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIO.h"
#include "cinder/Utilities.h"

#include "cinder/Camera.h"
#include "cinder/Arcball.h"

#include "GMLData.h"
#include "TagCollection.h"

#include "SimpleGUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace mowa::sgui;

const bool WRITE_FRAMES = false;

Vec3f light_pos;

//*************************************************************************************************************************
//*************************************************************************************************************************
class GrafAppApp : public AppBasic 
{
public:
	void setup();
	void keyDown(KeyEvent event);
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

	SimpleGUI* gui;
};

//*************************************************************************************************************************
void GrafAppApp::setup()
{	
	setWindowSize(1280, 1024);
	//setWindowSize(640, 480);
	m_Cam.lookAt(Vec3f(0, 0, -3), Vec3f::zero(), Vec3f(0, -1, 0));
	m_Cam.setNearClip(0.00001f);

	// Arcball
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( getWindowCenter() );
	mArcball.setRadius( 150 );
	//mArcball.setConstraintAxis(Vec3f(0.1,1,0));


	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();

	m_Rotation.setToIdentity();
	
	light_pos = Vec3f(0, 800, 3000);

	gui = new SimpleGUI(this);
	gui->lightColor = ColorA(1, 1, 0, 1);	
	gui->addLabel("CONTROLS");
	gui->addParam("BrushSize", &GrafDrawingParams::g_BrushSize, 0, 0.1f, GrafDrawingParams::g_BrushSize);
	gui->addParam("UpdateSpeed", &GrafDrawingParams::g_UpdateSpeed, 0, 0.5f, GrafDrawingParams::g_UpdateSpeed);
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
void GrafAppApp::keyDown(KeyEvent event)
{
	m_TagCollection.NextTag();
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
	m_TagCollection.Update();
	
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
	//gl::clear(Color(0,0,0));

	glColor3f(1,1,1);
	glColor3f(0,0,0);
	
	m_TagCollection.Draw();	
	
	glPopMatrix();
	
	
	//params::InterfaceGl::draw();
	gui->draw();

	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::disableDepthRead();	
	gl::disableDepthWrite();
	gl::enableAlphaBlending();

	glColor3f(1,1,1);
	float text_size = 24;
	float padding = text_size * 0.25f;
	gl::drawString(m_TagCollection.GetCurrTag().GetArtist(), Vec2f(padding, getWindowHeight() - text_size - padding), ColorA(0, 0, 0, 0.5), Font("Impact", text_size));

	gl::disableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color(ColorA(1,1,1,1));
	gl::popMatrices();

	if(WRITE_FRAMES)
	{
		writeImage( getHomeDirectory() + "Videos\\Graf\\" + "image_" + toString( getElapsedFrames() ) + ".png", copyWindowSurface() );
	}

}

CINDER_APP_BASIC( GrafAppApp, RendererGl )
