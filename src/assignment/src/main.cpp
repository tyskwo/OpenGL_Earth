#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>
#include <tlocInput/tloc_input.h>

#include <gameAssetsPath.h>

using namespace tloc;

//shader paths
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



//------------------------------------------------------------------------
// KeyboardCallback

class KeyboardCallback
{
public:
	KeyboardCallback() {}

	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	core_dispatch::Event
		OnKeyPress(const tl_size a_caller,
		const input_hid::KeyboardEvent& a_event)
	{
			TLOC_LOG_CORE_INFO() <<
				core_str::Format("Caller %i pressed a key. Key code is: %i",
				(tl_int)a_caller, a_event.m_keyCode);

			return core_dispatch::f_event::Continue();
		}

	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	core_dispatch::Event
		OnKeyRelease(const tl_size a_caller,
		const input_hid::KeyboardEvent& a_event)
	{
			TLOC_LOG_CORE_INFO() <<
				core_str::Format("Caller %i released a key. Key code is %i",
				(tl_int)a_caller, a_event.m_keyCode);

			return core_dispatch::f_event::Continue();
		}
};
TLOC_DEF_TYPE(KeyboardCallback);

//------------------------------------------------------------------------
// MouseCallback

class MouseCallback
{
public:
	MouseCallback()
	{ }

	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	core_dispatch::Event
		OnMouseButtonPress(const tl_size a_caller,
		const input_hid::MouseEvent&,
		const input_hid::MouseEvent::button_code_type a_button)
	{
			TLOC_LOG_CORE_INFO() <<
				core_str::Format("Caller %i pushed a button. Button state is: %i",
				(tl_int)a_caller, a_button);

			return core_dispatch::f_event::Continue();
		}

	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	core_dispatch::Event
		OnMouseButtonRelease(const tl_size a_caller,
		const input_hid::MouseEvent&,
		const input_hid::MouseEvent::button_code_type a_button)
	{
			TLOC_LOG_CORE_INFO() <<
				core_str::Format("Caller %i released a button. Button state is %i",
				(tl_int)a_caller, a_button);

			return core_dispatch::f_event::Continue();
		}

	// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	core_dispatch::Event
		OnMouseMove(const tl_size a_caller, const input_hid::MouseEvent& a_event)
	{
			TLOC_LOG_CORE_INFO() <<
				core_str::Format("Caller %i moved the mouse by %i %i %i ", (tl_int)a_caller,
				a_event.m_X.m_rel,
				a_event.m_Y.m_rel,
				a_event.m_Z.m_rel)
				<<
				core_str::Format(" to %i %i %i", a_event.m_X.m_abs,
				a_event.m_Y.m_abs,
				a_event.m_Z.m_abs);

			return core_dispatch::f_event::Continue();
		}
};
TLOC_DEF_TYPE(MouseCallback);





//----------------------------------------------------------------------------------
// assignment2 

class Assignment2 
  : public Application
{
public:
	Assignment2() : Application("NEWT | assignment2"),
		mAngleX(0.0f), mAngleY(0.0f),
		mScaleX(1.0f), mScaleY(1.0f), mScaleZ(1.0f),

		mIsAltPressed(false), mIsLeftMouseButtonPressed(false), mIsRightMouseButtonPressed(false), mIsCenterMouseButtonPressed(false)

	{}

  error_type Post_Initialize() override
  {
	  //change the clear color of the renderer
	  auto m_renderer = GetRenderer();
	  auto m_rendererParameters = m_renderer->GetParams();
	  m_rendererParameters.SetClearColor(gfx_t::Color(0.6f, 0.6f, 0.6f, 1.0f));

	  m_renderer->SetParams(m_rendererParameters);



	  //create the input manager, and initialize the keyboard and mouse.
	  ParamList<core_t::Any> params;
	  params.m_param1 = this->GetWindow()->GetWindowHandle();

	  mInputManager = core_sptr::MakeShared<input::InputManagerB>(params);
	  mKeyboard = mInputManager->CreateHID<input_hid::KeyboardB>();
	  mMouse = mInputManager->CreateHID<input_hid::MouseB>();


	  //check if there is a mouse and keyboard attached.
	  TLOC_LOG_CORE_WARN_IF(mKeyboard == nullptr) << "No keyboard detected";
	  TLOC_LOG_CORE_WARN_IF(mMouse == nullptr) << "No mouse detected";


	  //create keyboard and mouse callbacks and register them with their respective HIDs
	  KeyboardCallback keyboardCallback;
	  if (mKeyboard) { mKeyboard->Register(&keyboardCallback); }

	  MouseCallback mouseCallback;
	  if (mMouse) { mMouse->Register(&mouseCallback); }


	  return Application::Post_Initialize();
  }



private:

	//input manager, keyboard, mouse
	input::input_mgr_b_ptr		mInputManager;
	input_hid::keyboard_b_vptr  mKeyboard;
	input_hid::mouse_b_vptr     mMouse;


	//vertices
	math_t::Vec3f32 v1 = math_t::Vec3f32(-1,  1, -1),
					v2 = math_t::Vec3f32(-1, -1, -1),
					v3 = math_t::Vec3f32( 1, -1, -1),
					v4 = math_t::Vec3f32( 1,  1, -1),
					v5 = math_t::Vec3f32(-1,  1,  1),
					v6 = math_t::Vec3f32( 1,  1,  1),
					v7 = math_t::Vec3f32( 1, -1,  1),
					v8 = math_t::Vec3f32(-1, -1,  1);


	//colors
	math_t::Vec3f32 r = math_t::Vec3f32(0.9f, 0.1f, 0.2f),
					g = math_t::Vec3f32(0.2f, 0.9f, 0.1f),
					y = math_t::Vec3f32(0.9f, 0.9f, 0.1f),
					o = math_t::Vec3f32(0.9f, 0.7f, 0.1f),
					b = math_t::Vec3f32(0.1f, 0.2f, 0.9f),
					p = math_t::Vec3f32(0.7f, 0.1f, 0.9f);


  float mAngleX, mAngleY;			 //variables for rotation
  float mScaleX, mScaleY, mScaleZ;   //variables for scale

  //flags for input
  bool mIsAltPressed, mIsLeftMouseButtonPressed, mIsRightMouseButtonPressed, mIsCenterMouseButtonPressed;

  //constant to scale cube and keep vertice values clean
  math_t::Vec3f32 mConstantScaleFactor = math_t::Vec3f32(0.25f, 0.25f, 0.25f);



  void DoRender(sec_type) override
  {
	  //apply background/clear color
	  GetRenderer()->ApplyRenderSettings();

	  //need this so that the matrix transformation doesn't get reset each frame
	  glLoadIdentity();





//move this
	  //get aspect ratio of window
	  const float aspectRatio = core_utils::CastNumber<float>(GetWindow()->GetWidth()) / core_utils::CastNumber<float>(GetWindow()->GetHeight());

//will be moved and changed
	  //showing the ability to scale
	  glScalef(mConstantScaleFactor[0] * 1.0f / aspectRatio, mConstantScaleFactor[1], mConstantScaleFactor[2]);

	  //showing the ability to rotate
	  glRotatef(mAngleY++, 0, 1, 0); //rotate around the y axis
	  glRotatef(mAngleX++, 1, 0, 0); //rotate around the x axis
	  mAngleX++; //make it rotate on x axis twice as fast
//-----


//this will be in an update input method.
/*// This updates all HIDs attached to the InputManager. An InputManager can
// be updated at a different interval than the main render update.
inputMgr->Update();

// Polling if esc key is down, to exit program
if (keyboard && keyboard->IsKeyDown(input_hid::KeyboardEvent::escape))
{
exit(0);
}*/



	  //draw cube
	  glBegin(GL_TRIANGLES);
	  {
		  //---------------------
		  // blue side
		  glColor3fv(  b.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v2.data());
		  glVertex3fv(v4.data());

		  glVertex3fv(v2.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v4.data());
		  //---------------------

		  //---------------------
		  // red side
		  glColor3fv(  r.data());
		  glVertex3fv(v5.data());
		  glVertex3fv(v6.data());
		  glVertex3fv(v8.data());

		  glVertex3fv(v6.data());
		  glVertex3fv(v7.data());
		  glVertex3fv(v8.data());
		  //---------------------

		  //---------------------
		  // green side
		  glColor3fv(  g.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v6.data());

		  glVertex3fv(v3.data());
		  glVertex3fv(v7.data());
		  glVertex3fv(v6.data());
		  //---------------------

		  //---------------------
		  // yellow side
		  glColor3fv(  y.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v5.data());
		  glVertex3fv(v8.data());

		  glVertex3fv(v1.data());
		  glVertex3fv(v8.data());
		  glVertex3fv(v2.data());
		  //---------------------

		  //---------------------
		  // orange side
		  glColor3fv(  o.data());
		  glVertex3fv(v1.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v5.data());

		  glVertex3fv(v5.data());
		  glVertex3fv(v4.data());
		  glVertex3fv(v6.data());
		  //---------------------

		  //---------------------
		  // purple side
		  glColor3fv(  p.data());
		  glVertex3fv(v2.data());
		  glVertex3fv(v8.data());
		  glVertex3fv(v3.data());

		  glVertex3fv(v7.data());
		  glVertex3fv(v3.data());
		  glVertex3fv(v8.data());
		  //---------------------
	  }

	  //end the gl call.
	  glEnd();
  }
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

int TLOC_MAIN(int , char *[])
{
	//Create and start program.
	Assignment2 app;
	app.Initialize(core_ds::MakeTuple(800, 600));
	app.Run();

	//On exit.
	TLOC_LOG_CORE_INFO() << "Exiting normally";

	return 0;
}