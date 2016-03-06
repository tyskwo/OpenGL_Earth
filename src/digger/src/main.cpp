#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>

#include <tlocCore/io/tlocFileContents.h>


#include <gameAssetsPath.h>


//#include "Level.h"
#include <iostream>


//namespace
using namespace tloc;



//shader paths
namespace
{
	core_str::String shaderPathVS("/shaders/tlocSimpleLightingVS_perFragment.glsl");
	core_str::String shaderPathFS("/shaders/tlocSimpleLightingFS_perFragment.glsl");

	const core_str::String g_assetsPath(GetAssetsPath());
};



enum LevelInfo
{
	EMPTY = 0,
	DIRT = 1,
	EMERALD = 2,
	MONEY = 3,
	DIGGER_SPAWN = 4,
	ENEMY_SPAWN = 5
};


struct Tile
{
	bool isDug;
	math_t::Vec2f32 position;
	//TODO: add graphic field
};

class Level
{
public:
	inline int getGridWidth() { return GRID_WIDTH; };
	inline int getGridHeight() { return GRID_HEIGHT; };

	Level()
	{
		grid = new core_conts::Array<Tile>(NUM_TILES);
	}

	bool loadLevel(core_io::Path levelPath)
	{

		std::cout << "\n";

		float row = 0, col = 0;

		core_io::FileIO_ReadA levelFile(levelPath);

		if (levelFile.Open() != ErrorSuccess)
		{
			TLOC_LOG_CORE_INFO() << "Could not open " << levelPath;
			return false;
		}

		core_str::String levelFileContents;
		levelFile.GetContents(levelFileContents);

		for (auto iter = levelFileContents.begin(); iter != levelFileContents.end(); ++iter)
		{
			if (*iter == '\n')
			{
				std::cout << "\n";
				row++;
			}
			else if (*iter != ' ')
			{
				col++;
				Tile newTile;
				newTile.position = math_t::Vec2f32(col, row);

				switch (*iter)
				{
				case EMPTY:
					newTile.isDug = true;
					break;
				case DIRT:
					newTile.isDug = false;
					break;
				case EMERALD:
					newTile.isDug = false;
					//TODO: Add emerald entity at this location
					break;
				case MONEY:
					newTile.isDug = false;
					//TODO: Add MoneyBag entity at this location
					break;
				case DIGGER_SPAWN:
					newTile.isDug = false;
					//TODO: Add DiggerSpawn entity at this location
					break;
				case ENEMY_SPAWN:
					newTile.isDug = false;
					//TODO: Add EnemySpawn entity at this location
					break;
				default:
					newTile.isDug = false;
					break;
				}

				grid->push_back(newTile);

				std::cout << *iter << " ";
			}

		}

		return true;
	}


private:
	const int GRID_WIDTH = 15;
	const int GRID_HEIGHT = 10;
	const tl_size NUM_TILES = GRID_WIDTH * GRID_HEIGHT;

	//TODO front texture
	//TODO backTexture

	core_conts::Array<Tile>* grid;
};







/////////////////////////////////////////////////////////////////////////
// Digger

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
	MeshRenderSystem meshSystem; //the render system 3D meshes

	core_str::String sphereObjectPath = "/models/sphereSmooth.obj"; //the sphere (path to it)
	Entity sphereMesh; //the actual sphere

	gfx_gl::uniform_vso lightPosition; //position of the light


	Material* sphereMaterial;


	Level* currentLevel;


	//after calling the constructor
	error_type Post_Initialize() override
	{
		loadScene();

		core_str::String levelPath = "levelInfo/level_01.txt";

		currentLevel->loadLevel(getPath(levelPath));

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

		currentLevel = new Level();

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