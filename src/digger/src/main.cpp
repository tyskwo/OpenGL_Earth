#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>

#include <gameAssetsPath.h>


#include "Level.h"

using namespace tloc;



//shader paths
namespace
{
	core_str::String shaderPathVS("/shaders/tlocSimpleLightingVS_perFragment.glsl");
	core_str::String shaderPathFS("/shaders/tlocSimpleLightingFS_perFragment.glsl");

	const core_str::String g_assetsPath(GetAssetsPath());
};






/////////////////////////////////////////////////////////////////////////
// Digger

class Program : public Application
{
public:
	Program() : Application("digger") { }


private:

	

//typedefs
	typedef ecs_ptr																			Scene;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		RenderSystem;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
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
	Scene		 scene;			//the scene from the application
	RenderSystem renderSystem;	//the render system

	Object* sphere; //the actual sphere

	gfx_gl::uniform_vso lightPosition; //position of the light


	Material defaultMaterial;


	Level* currentLevel;


	//after calling the constructor
	error_type Post_Initialize() override
	{
		loadScene();


		core_str::String levelPath = "levelInfo/level_01.txt";
		currentLevel = new Level();
		currentLevel->loadLevel(getPath(levelPath));

		//create a default material and set the light position
		defaultMaterial = createMaterial(shaderPathVS, shaderPathFS);

		//initialize the sphere
		sphere = new Object(scene, "/models/torus.obj", defaultMaterial);

		return Application::Post_Initialize();
	}

//load the scene
	void loadScene()
	{
		scene = GetScene();
		scene->AddSystem<gfx_cs::MaterialSystem>();	//add material system
		scene->AddSystem<gfx_cs::CameraSystem>();	//add camera

		renderSystem = scene->AddSystem<gfx_cs::MeshRenderSystem>();	//add mesh render system

		//set renderer
		renderSystem->SetRenderer(GetRenderer());

		//create and set the camera
		renderSystem->SetCamera(createCamera());

		currentLevel = new Level();

		//set the light position
		setLightPosition();
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

//get the path of the given string
	core_io::Path getPath(core_str::String objectPath)
	{
		//get the path to the object file
		return core_io::Path(core_str::String(GetAssetsPath()) + objectPath);
			//any place you want to pass a string or const char, use a core_io::String (which is a BufferArg), converts to and from both.
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




/*
gfx_med::ImageLoaderPng png;
core_io::Path path((core_str::String(GetAssetsPath()) +
	"/images/uv_grid_col.png").c_str());

if (png.Load(path) != ErrorSuccess)
{
	TLOC_ASSERT_FALSE("Image did not load!");
}*/



/*
//load png, slap to quad through meshrendersystem

//texture packer or spritesheet packer

xmlPath = GetAssetsPath() + xmlPath;

core_io::FileIO_ReadA file((core_io::Path(xmlPath)));

if (file.Open() != ErrorSuccess)
{
	TLOC_LOG_GFX_ERR() << "Unable to open the sprite sheet";
}

gfx_med::SpriteLoader_TexturePacker ssp;
core_str::String                    sspContents;

file.GetContents(sspContents);
ssp.Init(sspContents, png.GetImage()->GetDimensions());

pref_gfx::SpriteAnimation(entityMgr.get(), cpoolMgr.get())
.Loop(true).Fps(24).Add(spriteEnt, ssp.begin(), ssp.end());*/