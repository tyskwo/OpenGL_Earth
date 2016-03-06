#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>



#include <tlocCore/io/tlocFileContents.h>


#include <gameAssetsPath.h>



//namespace
using namespace tloc;



//shader paths
namespace
{
	core_str::String shaderPathVS("/shaders/tlocSimpleLightingVS_perFragment.glsl");
	core_str::String shaderPathFS("/shaders/tlocSimpleLightingFS_perFragment.glsl");

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
	typedef pref_gfx::Material																Material;

	//variables
	Scene scene;				 //the scene from the application
	MeshRenderSystem meshSystem; //the render system

	core_str::String sphereObjectPath = "/models/sphereSmooth.obj"; //the sphere (path to it)
	Entity sphereMesh; //the actual sphere

	gfx_gl::uniform_vso lightPosition; //position of the light



	Material* sphereMaterial;



	//after calling the constructor
	error_type Post_Initialize() override
	{
		loadScene();

		sphereMesh = createMesh(sphereObjectPath);

		//CAN NOT MAKE THIS A CLASS MEMBER??
		sphereMaterial = createMaterial();

		//IS THERE A WAY I CAN PASS THESE INTO ^^^ AS AN OPTIONS PARAMETER OF SORTS?
		sphereMaterial->AddUniform(lightPosition.get());
		sphereMaterial->Add(sphereMesh, core_io::Path(shaderPathVS), core_io::Path(shaderPathFS));

		return Application::Post_Initialize();
	}





	//load the scene
	void loadScene()
	{
		scene = GetScene();
		scene->AddSystem<gfx_cs::MaterialSystem>();	//add material system
		scene->AddSystem<gfx_cs::CameraSystem>();		//add camera
		meshSystem = scene->AddSystem<gfx_cs::MeshRenderSystem>();	//add mesh render system	


		//set renderer
		meshSystem->SetRenderer(GetRenderer());

		//create and set the camera
		meshSystem->SetCamera(createCamera());

		//set the light position
		setLightPosition();
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
		return scene->CreatePrefab<pref_gfx::Mesh>().Create(loadObject(sphereObjectPath));
	}

	//create material
	Material* createMaterial()
	{
		Material* temp = new Material(scene->GetEntityManager(), scene->GetComponentPoolManager());

		//set the assets path of the material.
		temp->AssetsPath(GetAssetsPath());

		return temp;
	}

	//get the path of the given string
	core_io::Path getPath(core_str::String objectPath)
	{
		//get the path to the object file
		return core_io::Path(core_str::String(GetAssetsPath()) + objectPath);
		//any place you want to pass a string or const char, use a core_io::String (which is a BufferArg), converts to and from both.
	}

	//create a camera
	entity_ptr createCamera()
	{
		entity_ptr cameraEntity = scene->CreatePrefab<pref_gfx::Camera>()
			.Perspective(true)
			.Near(0.1f)
			.Far(100.0f)
			.VerticalFOV(math_t::Degree(60.0f))
			.Create(GetWindow()->GetDimensions());

		//change camera's position
		cameraEntity->GetComponent<math_cs::Transform>()->SetPosition(math_t::Vec3f32(0, 0, 5));

		return cameraEntity;
	}

	//set the shader's light positions
	void setLightPosition()
	{
		lightPosition->SetName("u_lightPosition").SetValueAs(math_t::Vec3f32(1.0f, 1.0f, 3.0f));
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