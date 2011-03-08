#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIO.h"
#include "cinder/Utilities.h"

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"

#include "GMLData.h"
#include "TagCollection.h"

#include "SimpleGUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace mowa::sgui;

const bool WRITE_FRAMES = false;

Vec3f light_pos;

float eye_z = 4;

float fog_start = eye_z;
float fog_end = eye_z + 3;
float fog_density = 0.3f;
ColorA fog_colour(1,1,1,1);

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
	MayaCamUI				m_MayaCam;

	SimpleGUI* gui;
};

void Assign(boost::shared_ptr<float>& p1, boost::shared_ptr<float>& p2)
{
	p1 = p2;
}

//*************************************************************************************************************************
void GrafAppApp::setup()
{	
	boost::shared_ptr<float> mySample(new float); 
	console() << mySample.use_count() << "\n"; // should be 1
	boost::shared_ptr<float> mySample2;
	Assign(mySample2, mySample); // should be 2 refs by now
	console() << mySample.use_count() << "\n"; // should be 2
	mySample.reset(); 
	console() << mySample.use_count() << "\n"; // should be 1
	console() << mySample2.use_count() << "\n"; // should be 1
	
	setWindowSize(1280, 1024);
	setWindowSize(640, 480);

	m_Cam.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	m_Cam.setCenterOfInterestPoint(Vec3f( 0, 0, 0 ));
	//Vec3f p = m_Cam.getEyePoint();
	//m_Cam.setEyePoint(Vec3f(10,10,10));

	m_Cam.lookAt(Vec3f(0, 0, -eye_z), Vec3f::zero(), Vec3f(0, -1, 0));
	//m_Cam.setNearClip(0.00001f);
	m_Cam.setCenterOfInterestPoint(Vec3f( 0, 0, 0 ));


	m_MayaCam.setCurrentCam(m_Cam);


	// Arcball
	//mArcball.setWindowSize( getWindowSize() );
	//mArcball.setCenter( getWindowCenter() );
	//mArcball.setRadius( 150 );
	//mArcball.setConstraintAxis(Vec3f(0.1,1,0));


	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();

	m_Rotation.setToIdentity();
	
	light_pos = Vec3f(0, 80, -80);

	gui = new SimpleGUI(this);
	gui->lightColor = ColorA(1, 1, 0, 1);	
	gui->addLabel("CONTROLS");
	gui->addQuickParam("BrushSize", &GrafDrawingParams::g_BrushSize);
	gui->addParam("UpdateSpeed", &GrafDrawingParams::g_UpdateSpeed, 0, 0.5f, GrafDrawingParams::g_UpdateSpeed);
	gui->addLabel("FOG");
	gui->addQuickParam("Start", &fog_start);
	gui->addQuickParam("End", &fog_end);
	gui->addQuickParam("Density", &fog_density);
	gui->addParam("Colour", &fog_colour, fog_colour, CM_HSV);

}

//*************************************************************************************************************************
void GrafAppApp::mouseDown(MouseEvent event)
{
	m_MayaCam.mouseDown(event.getPos());
}

//*************************************************************************************************************************
void GrafAppApp::mouseDrag(MouseEvent event)
{
	m_MayaCam.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
}

//*************************************************************************************************************************
void GrafAppApp::keyDown(KeyEvent event)
{
	switch(event.getCode())
	{
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen(!isFullScreen());
		break;
	case KeyEvent::KEY_r:
		m_TagCollection.ResetCurrent();
		break;
	case KeyEvent::KEY_n:
		m_TagCollection.FadeOut();
		break;
	}
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
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	//GLfloat fogColor[4] = {1.0f, 1.0f, 1.0f, 1.0};
	glFogfv(GL_FOG_COLOR, fog_colour);
	glFogf(GL_FOG_DENSITY, fog_density);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);

	//glCullFace(GL_FRONT);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix();
	
	Matrix44f trans;
	
	//gl::setMatrices(m_Cam);
	//gl::rotate(mArcball.getQuat());
	gl::setMatrices(m_MayaCam.getCamera());

	
	GLfloat light_position[] = { light_pos.x, light_pos.y, light_pos.z, true };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat ambient[] = { 0.2f, 0.2f, 0.5f, 1.0f };
	GLfloat diffuse[] = { 0.5f, 0.2f, 0.2f, 1.0f };
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse); 

	ci::ColorA color( CM_HSV, 0.0f, 1.0f, 1.0f, 1.0f );
	glMaterialfv( GL_FRONT, GL_DIFFUSE,	color );


	gl::clear(fog_colour);

	m_TagCollection.Draw();	
	
	glPopMatrix();
	


	glDisable(GL_LIGHTING);
	
	//params::InterfaceGl::draw();
	gui->draw();

	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::disableDepthRead();	
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	glDisable(GL_FOG);


	glColor3f(1,1,1);
	float text_size = 24;
	float padding = text_size * 0.25f;
	gl::drawStringRight(m_TagCollection.GetCurrTag().GetArtist(), Vec2f(getWindowWidth() - padding, getWindowHeight() - text_size - padding), ColorA(0, 0, 0, 0.5), Font("Impact", text_size));

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
