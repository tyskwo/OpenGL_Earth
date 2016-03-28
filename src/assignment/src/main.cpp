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
	core_str::String shaderPathVS("/shaders/globeShaderVS.glsl");
	core_str::String shaderPathFS("/shaders/globeShaderFS.glsl");

	const core_str::String g_assetsPath(GetAssetsPath());
};





/////////////////////////////////////////////////////////////////////////
// lighting sample

class Program : public Application
{
public:
	Program() : Application("lighting example") { }


private:

	//typedefs
	typedef ecs_ptr																			Scene;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		MeshRenderSystem;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
	typedef core::smart_ptr::VirtualPtr<input::component_system::ArcBallControlSystem>		ArcBallControlSystem;
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
		Object(Scene sceneReference, core_str::String filePath, Material materialReference)
		{
			objectPath = filePath;
			scene = sceneReference;

			mesh = createMesh(objectPath);
			material = materialReference;

			scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(mesh, material));
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
		Entity createMesh(core_str::String object)
		{
			return scene->CreatePrefab<pref_gfx::Mesh>().Create(loadObject(objectPath));
		}
	};


	//variables
	Scene					scene;			//the scene from the application
	MeshRenderSystem		meshSystem;		//the render system
	ArcBallControlSystem	cameraControl;	//the camera controls


	Material defaultMaterial; //the default material with per-fragment lighting

	core_conts::List<Object*> objects;
	Object* sphere; //the sphere

	gfx_gl::uniform_vso lightPosition; //position of the light



	//after calling the constructor
	error_type Post_Initialize() override
	{

		//load the scene
		loadScene();


		//create a default material and set the light position
		defaultMaterial = createMaterial(shaderPathVS, shaderPathFS);

		auto so = defaultMaterial->GetShaderOperator();
		auto texObj_1 = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earthmap1k.jpg")));
		auto texObj_2 = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earthspec1k.jpg")));

		gfx_gl::uniform_vso u_firstTexture;
		u_firstTexture->SetName("s_texture").SetValueAs(*texObj_1);

		gfx_gl::uniform_vso u_secondTexture;
		u_secondTexture->SetName("s_texture_2").SetValueAs(*texObj_2);

		so->AddUniform(*u_firstTexture);
		so->AddUniform(*u_secondTexture);



		//initialize the sphere
		sphere = new Object(scene, "/models/globe.obj", defaultMaterial);

		return Application::Post_Initialize();
	}

	//load the scene
	void loadScene()
	{
		scene = GetScene();
		scene->AddSystem<  gfx_cs::MaterialSystem>();		//add material system
		scene->AddSystem<  gfx_cs::CameraSystem>();			//add camera
		meshSystem = scene->AddSystem<  gfx_cs::MeshRenderSystem>();		//add mesh render system	
		scene->AddSystem<  gfx_cs::ArcBallSystem >();		//add the arc ball system
		cameraControl = scene->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system

		//set renderer
		meshSystem->SetRenderer(GetRenderer());

		//set the background color
		gfx_rend::Renderer::Params clearColor(GetRenderer()->GetParams());
		clearColor.SetClearColor(gfx_t::Color(0.5f, 0.5f, 1.0f, 1.0f));
		GetRenderer()->SetParams(clearColor);

		//create and set the camera
		meshSystem->SetCamera(createCamera(true, 0.1f, 100.0f, 90.0f, math_t::Vec3f32(0, 0, 15)));

		//set up the mouse and keyboard
		registerInputDevices();

		//set the light position
		setLightPosition(math_t::Vec3f32(1.0f, 1.0f, 3.0f));
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
			.AddUniform(lightPosition.get())
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
		lightPosition->SetName("u_lightPosition").SetValueAs(position);
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