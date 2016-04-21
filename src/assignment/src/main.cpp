#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>
#include <tlocInput/tloc_input.h>

#include <gameAssetsPath.h>

using namespace tloc;



//shader paths
namespace
{
	core_str::String shaderPathVS("/shaders/sphereVS.glsl");
	core_str::String shaderPathFS("/shaders/sphereFS.glsl");

	core_str::String textureVS("/shaders/textureVS.glsl");
	core_str::String textureFS("/shaders/textureFS.glsl");

	core_str::String bloomVS("/shaders/bloomVS.glsl");
	core_str::String bloomFS("/shaders/bloomFS.glsl");
};





/////////////////////////////////////////////////////////////////////////
// lighting sample

class Program : public Application
{
public:
	Program() : Application("bloom") { }


private:

	//typedefs
	typedef ecs_ptr																			Scene;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		MeshRenderSystem;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
	typedef core::smart_ptr::VirtualPtr<input::component_system::ArcBallControlSystem>		ArcBallControlSystem;
	typedef core::smart_ptr::SharedPtr<tloc::graphics::gl::TextureObject>					TextureObject;
	typedef gfx_cs::material_sptr 															Material;


	//struct for a 3D object
	struct Object
	{
	private:
		Scene				scene;			//reference to the scene
		core_str::String	objectPath;		//the path to the obj file
		Entity				mesh;			//the actual object
		Material			material;		//the material of the object

	public:
		//intialize and create the object
		Object(Scene sceneReference, MeshRenderSystem meshSystem, core_str::String filePath, Material materialReference)
		{
			objectPath = filePath;
			scene = sceneReference;

			mesh = createMesh(objectPath, meshSystem);
			material = materialReference;

			scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(mesh, material));

			setMatrix();
		}

		//getters for the mesh and material
		Entity	 GetMesh()		{ return mesh; }
		Material GetMaterial()  { return material; }

		//get the path of the given string
		core_io::Path getPath(core_str::String objectPath)
		{
			//get the path to the object file
			return core_io::Path(core_str::String(GetAssetsPath()) + objectPath);
			//any place you want to pass a string or const char, use a core_io::String (which is a BufferArg), converts to and from both.
		}
		//load the passed object
		gfx_med::ObjLoader::vert_cont_type loadObject(core_str::String object)
		{
			//open up the .obj file, and report error if necessary
			core_io::FileIO_ReadA objFile(getPath(object));
			if (objFile.Open() != ErrorSuccess)
			{
				TLOC_LOG_GFX_ERR() << "Could not open " << getPath(object);
			}


			//get contents of the .obj file
			core_str::String objFileContents;
			objFile.GetContents(objFileContents);


			//try loading the object, and check for parsing errors
			gfx_med::ObjLoader objLoader;
			if (objLoader.Init(objFileContents) != ErrorSuccess)
			{
				TLOC_LOG_GFX_ERR() << "Failed to parse .obj file.";
			}


			//vertices of the object
			gfx_med::ObjLoader::vert_cont_type vertices;

			//unpack the vertices of the object
			{ objLoader.GetUnpacked(vertices, 0); }


			return vertices;
		}

		//create mesh from given object path
		Entity createMesh(core_str::String object, MeshRenderSystem meshSystem)
		{
			return scene->CreatePrefab<pref_gfx::Mesh>().DispatchTo(meshSystem.get()).Create(loadObject(objectPath));
		}

		void setMatrix()
		{
			mesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>();
			mesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();

			mesh->GetComponent<gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_modelMatrix>();
			mesh->GetComponent<gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_normalMatrix>();
		}
	};


	//variables
	Scene					scene;			//the scene from the application
	MeshRenderSystem		meshSystem,		//the render systems
		                    quadSystem;
	ArcBallControlSystem	cameraControl;	//the camera controls


	Material defaultMaterial, //the materials
		     textureMaterial,
			 bloomMaterial; 

	Object* sphere; //the sphere

	core_cs::entity_vptr splitQuad;


	gfx::rtt_sptr rtt;
	TextureObject rtt_normalColors, rtt_brightColors;



	//after calling the constructor
	error_type Post_Initialize() override
	{
		//load the scene
		loadScene();

		//create a default material and set the light position
		defaultMaterial = createMaterial(shaderPathVS, shaderPathFS);
		textureMaterial = createMaterial(textureVS,    textureFS);
		bloomMaterial   = createMaterial(bloomVS,      bloomFS);


		//set the light position
		setLightPosition(math_t::Vec3f32(0.0f, 0.0f, 1.0f));
		setNormalColorTexture();
		setBrightColorTexture();

		//initialize the sphere
		sphere = new Object(scene, meshSystem, "/models/globe.obj", defaultMaterial);


		//create rectangle
		math_t::Rectf_c rect(math_t::Rectf_c::width(GetWindow()->GetAspectRatio().Get() * 2.0f),
			math_t::Rectf_c::height(GetWindow()->GetAspectRatio().Get() * 2.0f));


		splitQuad = scene->CreatePrefab<pref_gfx::Quad>()
			.DispatchTo(quadSystem.get())
			.Dimensions(rect).Create();

		core_cs::entity_vptr bloomQuad = scene->CreatePrefab<pref_gfx::Quad>()
			.DispatchTo(quadSystem.get())
			.Dimensions(rect).Create();

		//core_cs::entity_vptr finalQuad = scene->CreatePrefab<pref_gfx::Quad>()
			//.DispatchTo(quadSystem.get())
			//.Dimensions(rect).Create();


		scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(splitQuad, textureMaterial));
		scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(bloomQuad, bloomMaterial));
		//scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(finalQuad, textureMaterial));



		return Application::Post_Initialize();
	}

	//load the scene
	void loadScene()
	{
		rtt = core_sptr::MakeShared<gfx::Rtt>(core_ds::MakeTuple(1024, 1024));
		rtt_normalColors = rtt->AddColorAttachment<0, gfx_t::color_u16_rgba>();
		rtt_brightColors = rtt->AddColorAttachment<1, gfx_t::color_u16_rgba>();

		auto toRttParams = rtt_normalColors->GetParams();
		toRttParams.MinFilter<gfx_gl::p_texture_object::filter::Nearest>();
		toRttParams.MagFilter<gfx_gl::p_texture_object::filter::Nearest>();

		rtt_normalColors->SetParams(toRttParams);
		rtt_brightColors->SetParams(toRttParams);
		rtt_normalColors->UpdateParameters();
		rtt_brightColors->UpdateParameters();

		rtt->AddDepthAttachment();
		auto rttRend = rtt->GetRenderer();



		scene = GetScene();
			     	    scene->AddSystem<  gfx_cs::MaterialSystem      >();	//add material system
					    scene->AddSystem<  gfx_cs::CameraSystem        >();	//add camera
		meshSystem =    scene->AddSystem<  gfx_cs::MeshRenderSystem    >();	//add mesh render system	
		quadSystem =    scene->AddSystem<  gfx_cs::MeshRenderSystem    >();
					    scene->AddSystem<  gfx_cs::ArcBallSystem       >();	//add the arc ball system
		cameraControl = scene->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system

	//set renderer
		meshSystem->SetRenderer(rttRend);
		quadSystem->SetRenderer(GetRenderer());

	//create and set the camera
		meshSystem->SetCamera(createCamera(true, 0.1f, 100.0f, 90.0f, math_t::Vec3f32(0, 0, 5.0f)));

	//set up the mouse and keyboard
		registerInputDevices();
	}

//create a camera
	entity_ptr createCamera(bool isPerspectiveView, float nearPlane, float farPlane, float verticalFOV_degrees, math_t::Vec3f32 position)
	{
		entity_ptr cameraEntity = scene->CreatePrefab<pref_gfx::Camera>()
			.Perspective(isPerspectiveView)
			.Near(nearPlane)
			.Far(farPlane)
			.VerticalFOV(math_t::Degree(verticalFOV_degrees))
			.Create(GetWindow()->GetDimensions());

		//add the camera to the arcball system
		scene->CreatePrefab<pref_gfx::ArcBall>().Add(cameraEntity);
		scene->CreatePrefab<pref_input::ArcBallControl>()
			.GlobalMultiplier(math_t::Vec2f(0.01f, 0.01f))
			.Add(cameraEntity);

		//change camera's position
		cameraEntity->GetComponent<math_cs::Transform>()->SetPosition(position);

		return cameraEntity;
	}
//create material
	Material createMaterial(core_str::String vertexShader, core_str::String fragmentShader)
	{
		auto materialEntity = scene->CreatePrefab<pref_gfx::Material>()
			.AssetsPath(GetAssetsPath())
			.Create(core_io::Path(vertexShader), core_io::Path(fragmentShader));

		return materialEntity->GetComponent<gfx_cs::Material>();
	}
//create the mouse and keyboard
	void registerInputDevices()
	{
		GetKeyboard()->Register(&*cameraControl);
		GetMouse()->Register(&*cameraControl);
	}

//set the shader's light positions
	void setLightPosition(math_t::Vec3f32 position)
	{
		gfx_gl::uniform_vso u_lightPosition; u_lightPosition->SetName("u_lightPosition").SetValueAs(position);

		defaultMaterial->GetShaderOperator()->AddUniform(*u_lightPosition);
	}

	void setNormalColorTexture()
	{
		gfx_gl::uniform_vso u_normal; u_normal->SetName("texture").SetValueAs(*rtt_normalColors);

		textureMaterial->GetShaderOperator()->AddUniform(*u_normal);
	}
	void setBrightColorTexture()
	{
		gfx_gl::uniform_vso u_normal; u_normal->SetName("texture").SetValueAs(*rtt_brightColors);

		bloomMaterial->GetShaderOperator()->AddUniform(*u_normal);
	}

	void Pre_Render(sec_type) override
	{
		//scene->GetEntityManager()->DeactivateEntity(splitQuad);
		rtt->GetRenderer()->ApplyRenderSettings();
		rtt->GetRenderer()->Render();
	}
};





//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// main method
int TLOC_MAIN(int, char *[])
{
	//declare and initialize the program
	Program program;
	program.Initialize(core_ds::MakeTuple(800, 600));

	//run the program
	program.Run();

	//on exit
	TLOC_LOG_CORE_INFO() << "Exiting normally";

	return 0;
}
