#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>

#include <gameAssetsPath.h>

using namespace tloc;

namespace {

#if defined (TLOC_OS_WIN)
    core_str::String shaderPathVS("/shaders/tlocOneTextureVS.glsl");
#elif defined (TLOC_OS_IPHONE)
    core_str::String shaderPathVS("/shaders/tlocOneTextureVS_gl_es_2_0.glsl");
#endif

#if defined (TLOC_OS_WIN)
    core_str::String shaderPathFS("/shaders/tlocOneTextureFS.glsl");
#elif defined (TLOC_OS_IPHONE)
    core_str::String shaderPathFS("/shaders/tlocOneTextureFS_gl_es_2_0.glsl");
#endif

  const core_str::String g_assetsPath(GetAssetsPath());

};

// ///////////////////////////////////////////////////////////////////////
// Demo app

class Demo 
  : public Application
{
public:
	Demo() : Application("NEWT | assignment2"),
		mAngleX(0.0f),
		mAngleY(0.0f)
  { }

  error_type Post_Initialize() override
  {
	  auto m_renderer = GetRenderer();
	  auto m_rendererParameters = m_renderer->GetParams();
	  m_rendererParameters.SetClearColor(gfx_t::Color(0.6f, 0.6f, 0.6f, 1.0f));

	  m_renderer->SetParams(m_rendererParameters);

	  return Application::Post_Initialize();
  }

private:
  //variables for the rotation amount around the x and y axis
  float mAngleX, mAngleY;

  void DoRender(sec_type) override
  {
	  //vertices
	  math_t::Vec3f32 v1(-1,  1, -1),
					  v2(-1, -1, -1),
					  v3( 1, -1, -1),
					  v4( 1,  1, -1),

					  v5(-1,  1, 1),
					  v6( 1,  1, 1),
					  v7( 1, -1, 1),
					  v8(-1, -1, 1);

	  //colors
	  math_t::Vec3f32 r(0.9f, 0.1f, 0.2f),
					  g(0.2f, 0.9f, 0.1f),
					  y(0.9f, 0.9f, 0.1f),
					  o(0.9f, 0.7f, 0.1f),
					  b(0.1f, 0.2f, 0.9f),
					  p(0.7f, 0.1f, 0.9f);

	  //apply background/clear color
	  GetRenderer()->ApplyRenderSettings();


	  //get aspect ratio of window
	  const float aspectRatio = core_utils::CastNumber<float>(GetWindow()->GetWidth()) / core_utils::CastNumber<float>(GetWindow()->GetHeight());




	  //need this so that the matrix transformation doesn't get reset each frame
	  glLoadIdentity();




	  //showing the ability to scale
	  math_t::Vec3f32 scaleFactor(0.5f, 0.5f, 0.5f);
	  glScalef(scaleFactor[0] * 1.0f / aspectRatio, scaleFactor[1], scaleFactor[2]);

	  //showing the ability to rotate
	  glRotatef(mAngleY++, 0, 1, 0); //rotate around the y axis
	  glRotatef(mAngleX++, 1, 0, 0); //rotate around the x axis
	  mAngleX++; //make it rotate on x axis twice as fast



	  //draw cube
	  glBegin(GL_TRIANGLES);
	  {
		  //---------------------
		  // blue side
		  glColor3fv(b.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v2.data());
		  glVertex3fv(v4.data());

		  glVertex3fv(v2.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v4.data());
		  //---------------------

		  //---------------------
		  // red side
		  glColor3fv(r.data());
		  glVertex3fv(v5.data());
		  glVertex3fv(v6.data());
		  glVertex3fv(v8.data());

		  glVertex3fv(v6.data());
		  glVertex3fv(v7.data());
		  glVertex3fv(v8.data());
		  //---------------------

		  //---------------------
		  // green side
		  glColor3fv(g.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v6.data());

		  glVertex3fv(v3.data());
		  glVertex3fv(v7.data());
		  glVertex3fv(v6.data());
		  //---------------------

		  //---------------------
		  // yellow side
		  glColor3fv(y.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v5.data());
		  glVertex3fv(v8.data());

		  glVertex3fv(v1.data());
		  glVertex3fv(v8.data());
		  glVertex3fv(v2.data());
		  //---------------------

		  //---------------------
		  // orange side
		  glColor3fv(o.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v5.data());

		  glVertex3fv(v5.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v6.data());
		  //---------------------

		  //---------------------
		  // purple side
		  glColor3fv(p.data());
		  glVertex3fv(v2.data());
		  glVertex3fv(v8.data());
		  glVertex3fv(v3.data());

		  glVertex3fv(v7.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v8.data());
		  //---------------------
	  }
	  glEnd();
  }
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

int TLOC_MAIN(int , char *[])
{
  Demo demo;
  demo.Initialize(core_ds::MakeTuple(800, 600));
  demo.Run();

  //------------------------------------------------------------------------
  // Exiting
  TLOC_LOG_CORE_INFO() << "Exiting normally";

  return 0;

}
