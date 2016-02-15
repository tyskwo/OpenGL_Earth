#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>
#include <tlocInput/tloc_input.h>

#include <gameAssetsPath.h>

#include <memory>
#include <vector>



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


//tyepdef for vertex list
typedef std::vector<math_t::Vec3f32> VertexList;



//----------------------------------------------------------------------------------
// assignment2 

class Assignment2 : public Application
{
public:
	Assignment2() : Application("NEWT | assignment2"),
		mAngleX(0.0f), mAngleY(0.0f),
		mZoomFactor(0.0f),
		mTranslation(0.0f, 0.0f, 0.0f),
		mScaleFactor(mBaseScaleFactor)
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
		mKeyboard	  = mInputManager->CreateHID<input_hid::KeyboardB>();
		mMouse	      = mInputManager->CreateHID<input_hid::MouseB>();



		//check if there is a mouse and keyboard attached.
		TLOC_LOG_CORE_WARN_IF(mKeyboard == nullptr) << "No keyboard detected";
		TLOC_LOG_CORE_WARN_IF(mMouse    == nullptr) << "No mouse detected";

		//set aspect ratio for correct perspective
		mWindowAspectRatio = core_utils::CastNumber<float>(GetWindow()->GetWidth()) / core_utils::CastNumber<float>(GetWindow()->GetHeight());



		return Application::Post_Initialize();
	}



private:

	//input manager, keyboard, mouse
	input::input_mgr_b_ptr		mInputManager;
	input_hid::keyboard_b_vptr  mKeyboard;
	input_hid::mouse_b_vptr     mMouse;

	//vertices
	VertexList vertices;

	//colors
	math_t::Vec3f32 r = math_t::Vec3f32(0.9f, 0.1f, 0.2f),
					g = math_t::Vec3f32(0.2f, 0.9f, 0.1f),
					y = math_t::Vec3f32(0.9f, 0.9f, 0.1f),
					o = math_t::Vec3f32(0.9f, 0.7f, 0.1f),
					b = math_t::Vec3f32(0.1f, 0.2f, 0.9f),
					p = math_t::Vec3f32(0.7f, 0.1f, 0.9f);


	//cube movement variables
	float			mAngleX, mAngleY; //variables for rotation
	float			mZoomFactor;	  //variable  for zoom
	math_t::Vec3f32 mTranslation;	  //variable  for translation


	
	//constant to scale cube and keep vertice values clean
	 math_t::Vec3f32 mBaseScaleFactor = math_t::Vec3f32(0.25f, 0.25f, 0.25f);

	//variable for current scale values
	math_t::Vec3f32 mScaleFactor;


	//value for aspect ratio of the window
	float mWindowAspectRatio;




	//--------------------------------------------------------------------------------
	// called each render call
	void DoRender(sec_type) override
	{
		//apply background/clear color
		GetRenderer()->ApplyRenderSettings();

		//need this so that the matrix transformation doesn't get reset each frame
		glLoadIdentity();




		//translate cube
		glTranslatef(mTranslation[0], mTranslation[1], 0.0f);

		//change scale
		glScalef((mScaleFactor[0] + mZoomFactor) * 1.0f / mWindowAspectRatio, mScaleFactor[1] + mZoomFactor, mScaleFactor[2] + mZoomFactor);

		//change rotation
		glRotatef(mAngleY, 0, 1, 0); //rotate around the y axis
		glRotatef(mAngleX, 1, 0, 0); //rotate around the x axis
	



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




	
	//--------------------------------------------------------------------------------
	// called each update
	void DoUpdate(sec_type) override
	{
		//check input from HIDs
		CheckInput();

		//if escape key is pressed, exit program
		if (mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::escape))
		{
			exit(0);
		}
	}





	//--------------------------------------------------------------------------------
	// receive update events, and change values accordingly
	void CheckInput()
	{
		//update input manager
		mInputManager->Update();

		//get current mouse state
		input_hid::MouseEvent currentMouseState = mMouse->GetState();

		//check to see if either of the control buttons are pressed
		if (mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::left_control) || 
			mKeyboard && mKeyboard->IsKeyDown(input_hid::KeyboardEvent::right_control))
		{
			//left mouse button --- rotate
			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::left))
			{
				mAngleX -= core_utils::CastNumber<tl_float>(currentMouseState.m_Y.m_rel);
				mAngleY -= core_utils::CastNumber<tl_float>(currentMouseState.m_X.m_rel);
			}

			//right mouse button --- zoom
			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::right))
			{
				mZoomFactor -= core_utils::CastNumber<tl_float>(currentMouseState.m_Y.m_rel) / 100.0f;

				//check to make sure we're not negatively scaled.
				if (mZoomFactor <= -mBaseScaleFactor[0]) mZoomFactor = -mBaseScaleFactor[0] + 0.01f;
			}

			//middle mouse button --- translate
			if (mMouse && mMouse->IsButtonDown(input_hid::MouseEvent::middle))
			{
				mTranslation[0] += core_utils::CastNumber<tl_float>(currentMouseState.m_X.m_rel) / 100.0f;
				mTranslation[1] -= core_utils::CastNumber<tl_float>(currentMouseState.m_Y.m_rel) / 100.0f;


				//bounds checking
				if (mTranslation[0] < -1.0f) mTranslation[0] = -1.0f;
				if (mTranslation[0] >  1.0f) mTranslation[0] =  1.0f;

				if (mTranslation[1] < -1.0f) mTranslation[1] = -1.0f;
				if (mTranslation[1] >  1.0f) mTranslation[1] =  1.0f;
			}
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