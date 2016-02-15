#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>
#include <tlocInput/tloc_input.h>
#include <vector>
#include <gameAssetsPath.h>
#include <memory>

using namespace tloc;


//----------------------------------------------------------------------------------
// shader paths
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

typedef std::vector<math_t::Vec3f32> VertexList;

//----------------------------------------------------------------------------------
// MouseCallback
class MouseCallback
{
public:
	MouseCallback() {}

	//on mouse movement
	core_dispatch::Event OnMouseMove(const tl_size a_caller, const input_hid::MouseEvent& a_event)
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

class Assignment2 : public Application
{
public:
	Assignment2() : Application("NEWT | assignment2"),
		mAngleX(0.0f), mAngleY(0.0f),
		mZoomed(false),
		mScaleFactor(mBaseScaleFactor),
		mPreviousMousePos(0.0f, 0.0f)
	{
		vertices.push_back(math_t::Vec3f32(-1,  1, -1));
		vertices.push_back(math_t::Vec3f32(-1, -1, -1));
		vertices.push_back(math_t::Vec3f32( 1, -1, -1));
		vertices.push_back(math_t::Vec3f32( 1,  1, -1));
		vertices.push_back(math_t::Vec3f32(-1,  1,  1));
		vertices.push_back(math_t::Vec3f32( 1,  1,  1));
		vertices.push_back(math_t::Vec3f32( 1, -1,  1));
		vertices.push_back(math_t::Vec3f32(-1, -1,  1));
	}

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
		TLOC_LOG_CORE_WARN_IF(mMouse    == nullptr) << "No mouse detected";



		//register mouse callback
		if (mMouse) { mMouse->Register(&mMouseCallback); }



		return Application::Post_Initialize();
	}



private:

	//input manager, keyboard, mouse
	input::input_mgr_b_ptr		mInputManager;
	input_hid::keyboard_b_vptr  mKeyboard;
	input_hid::mouse_b_vptr     mMouse;

	MouseCallback mMouseCallback;

	//vertices
	VertexList vertices;

	//colors
	math_t::Vec3f32 r = math_t::Vec3f32(0.9f, 0.1f, 0.2f),
					g = math_t::Vec3f32(0.2f, 0.9f, 0.1f),
					y = math_t::Vec3f32(0.9f, 0.9f, 0.1f),
					o = math_t::Vec3f32(0.9f, 0.7f, 0.1f),
					b = math_t::Vec3f32(0.1f, 0.2f, 0.9f),
					p = math_t::Vec3f32(0.7f, 0.1f, 0.9f);

	math_t::Vec2f mPreviousMousePos;

	float mAngleX, mAngleY;			 //variables for rotation
	bool mZoomed;


	
	//aspect ratio
	const float aspectRatio = core_utils::CastNumber<float>(GetWindow()->GetWidth()) / core_utils::CastNumber<float>(GetWindow()->GetHeight());

	//constant to scale cube and keep vertice values clean
	 math_t::Vec3f32 mBaseScaleFactor = math_t::Vec3f32(0.25f, 0.25f, 0.25f);

	//variales for Zoom scale
	 math_t::Vec3f32 mZoomScaleFactor = math_t::Vec3f32(0.25f, 0.25f, 0.25f);

	//variable for current scale values
	math_t::Vec3f32 mScaleFactor;





	void CheckInput()
	{
		//update input manager
		mInputManager->Update();

		input_hid::MouseEvent currentMouseState = mMouse->GetState();

		if (mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::left_control) || mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::right_control))
		{
			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::left))
			{
				/*tl_float xScaled = core_utils::CastNumber<tl_float>
					(currentMouseState.m_X.m_abs);
				tl_float yScaled = core_utils::CastNumber<tl_float>
					(currentMouseState.m_Y.m_abs);

				xScaled /= core_utils::CastNumber<tl_float>(mWidth);
				yScaled /= core_utils::CastNumber<tl_float>(mHeight);

				xScaled = (xScaled * 2) - 1;
				yScaled = (yScaled * 2) - 1;

				// mouse co-ords start from top-left, OpenGL from bottom-left
				yScaled *= -1;*/


				TLOC_LOG_CORE_INFO() <<
					core_str::Format(" LEFT MOUSE IS PRESSED");
			}

			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::right))
			{
				TLOC_LOG_CORE_INFO() <<
					core_str::Format(" RIGHT MOUSE IS PRESSED");
			}

			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::middle))
			{
				if (mZoomed)
				{
					mScaleFactor -= mZoomScaleFactor;
				}
				else
				{
					mScaleFactor += mZoomScaleFactor;
				}

				//toggle zoom
				mZoomed = !mZoomed;
			}
		}
	}




	void DoRender(sec_type) override
	{
		//apply background/clear color
		GetRenderer()->ApplyRenderSettings();

		//need this so that the matrix transformation doesn't get reset each frame
		glLoadIdentity();





	//move this
		

	//will be moved and changed
		//showing the ability to scale
		glScalef(mScaleFactor[0] * 1.0f / aspectRatio, mScaleFactor[1], mScaleFactor[2]);

		//showing the ability to rotate
		glRotatef(mAngleY++, 0, 1, 0); //rotate around the y axis
		glRotatef(mAngleX++, 1, 0, 0); //rotate around the x axis
		mAngleX++; //make it rotate on x axis twice as fast
	//-----





		//draw cube
		glBegin(GL_TRIANGLES);
		{
			//---------------------
			// blue side
			glColor3fv(  b.data());
			glVertex3fv(vertices[0].data());
			glVertex3fv(vertices[1].data());
			glVertex3fv(vertices[3].data());

			glVertex3fv(vertices[1].data());
			glVertex3fv(vertices[2].data());
			glVertex3fv(vertices[3].data());
			//---------------------

			//---------------------
			// red side
			glColor3fv(  r.data());
			glVertex3fv(vertices[4].data());
			glVertex3fv(vertices[5].data());
			glVertex3fv(vertices[7].data());

			glVertex3fv(vertices[5].data());
			glVertex3fv(vertices[6].data());
			glVertex3fv(vertices[7].data());
			//---------------------

			//---------------------
			// green side
			glColor3fv(  g.data());
			glVertex3fv(vertices[3].data());
			glVertex3fv(vertices[2].data());
			glVertex3fv(vertices[5].data());

			glVertex3fv(vertices[2].data());
			glVertex3fv(vertices[6].data());
			glVertex3fv(vertices[5].data());
			//---------------------

			//---------------------
			// yellow side
			glColor3fv(  y.data());
			glVertex3fv(vertices[0].data());
			glVertex3fv(vertices[4].data());
			glVertex3fv(vertices[7].data());

			glVertex3fv(vertices[0].data());
			glVertex3fv(vertices[7].data());
			glVertex3fv(vertices[1].data());
			//---------------------

			//---------------------
			// orange side
			glColor3fv(  o.data());
			glVertex3fv(vertices[0].data());
			glVertex3fv(vertices[3].data());
			glVertex3fv(vertices[4].data());

			glVertex3fv(vertices[4].data());
			glVertex3fv(vertices[3].data());
			glVertex3fv(vertices[5].data());
			//---------------------

			//---------------------
			// purple side
			glColor3fv(  p.data());
			glVertex3fv(vertices[1].data());
			glVertex3fv(vertices[7].data());
			glVertex3fv(vertices[2].data());

			glVertex3fv(vertices[6].data());
			glVertex3fv(vertices[2].data());
			glVertex3fv(vertices[7].data());
			//---------------------
		}

		//end the gl call.
		glEnd();
	}




	void DoUpdate(sec_type a_deltaT) override
	{
		sec_type deltaT = a_deltaT;

		deltaT *= 1;

		CheckInput();

		//if escape key is pressed, exit program
		if (mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::escape))
		{
			exit(0);
		}
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