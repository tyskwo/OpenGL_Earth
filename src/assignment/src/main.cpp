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
	core_str::String globeVS("/shaders/sphereVS.glsl");
	core_str::String globeFS("/shaders/sphereFS.glsl");

	core_str::String splitVS("/shaders/splitVS.glsl");
	core_str::String splitFS("/shaders/splitFS.glsl");

	core_str::String bloomVS("/shaders/bloomVS.glsl");
	core_str::String bloomFS("/shaders/bloomFS.glsl");

	core_str::String combineVS("/shaders/combineVS.glsl");
	core_str::String combineFS("/shaders/combineFS.glsl");
};





/////////////////////////////////////////////////////////////////////////
// BLOOM 

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



//struct for a ping pong rendering system using rtt and their associated texture objects with quads.
	struct PingPongRenderSystem
	{
	private:
		Scene			 scene;				//reference to the scene
		MeshRenderSystem quadSystem;		//the render system

		core_cs::entity_vptr quads[3];		//the quads to render to

		gfx::rtt_sptr renderObjects[2];		//the render to texture objects
		TextureObject textureObjects[3];	//the texture objects of ^^^
		
	public:
	//initialize the system
		PingPongRenderSystem(Scene sceneReference, MeshRenderSystem meshSystem, math_t::Rectf_c screenSize)
		{
		//set up references
			scene	   = sceneReference;
			quadSystem = meshSystem;

		//create the render and texture objects
			createRenderObjects();
			createTextureObjects();
		//set the parameters of the renderer
			setParams();

		//create the quads given the scene size
			createQuads(screenSize);
		}

	//getters to retun the first or second of the ping pong renderers
		gfx::rtt_sptr getFirstRenderObject()  { return renderObjects[0]; }
		gfx::rtt_sptr getSecondRenderObject() { return renderObjects[1]; }

	//return the quad system
		MeshRenderSystem getQuadSystem() { return quadSystem; }

	//get a quad or texture object at a specific index
		core_cs::entity_vptr getQuad(int index) { return quads[index]; }
		TextureObject		 getTO(int index)   { return textureObjects[index]; }


	//create the render objects
		void createRenderObjects()
		{
			for (int i = 0; i < 2; i++) 
			{
			//1024x1024 for default, larger produces more detail, smaller is more pixelated
				renderObjects[i] = core_sptr::MakeShared<gfx::Rtt>(core_ds::MakeTuple(1024, 1024));
				renderObjects[i]->AddDepthAttachment();
			}
		}
	//create the texture objects
		void createTextureObjects()
		{
			/* TODO: little janky, lots of magic numbers*/
			textureObjects[0] = renderObjects[0]->AddColorAttachment<0, gfx_t::color_u16_rgba>();
			textureObjects[1] = renderObjects[0]->AddColorAttachment<1, gfx_t::color_u16_rgba>();
			textureObjects[2] = renderObjects[1]->AddColorAttachment<0, gfx_t::color_u16_rgba>();
		}
	//create the quads given the screensize
		void createQuads(math_t::Rectf_c screenSize)
		{
			for (int i = 0; i < 3; i++) { quads[i] = scene->CreatePrefab<pref_gfx::Quad>().DispatchTo(quadSystem.get()).Dimensions(screenSize).Create(); }
		}
	//set the RTT parameters
		void setParams()
		{
			auto toRttParams = textureObjects[0]->GetParams();
			toRttParams.MinFilter<gfx_gl::p_texture_object::filter::Nearest>();
			toRttParams.MagFilter<gfx_gl::p_texture_object::filter::Nearest>();

			for (int i = 0; i < 3; i++)
			{
				textureObjects[i]->SetParams(toRttParams);
				textureObjects[i]->UpdateParameters();
			}
		}

		void render()
		{
			quadSystem->GetRenderer()->ApplyRenderSettings();
			quadSystem->GetRenderer()->Render();
		}

	//setters for the renderer, given an index for a render object or to an outside renderer
		void setRenderer(int index)								   { quadSystem->SetRenderer(renderObjects[index]->GetRenderer()); }
		void setRenderer(tloc::Application::renderer_ptr renderer) { quadSystem->SetRenderer(renderer); }

	//render a quad at the given index
		void RenderEntity(int index, sec_type timeDelta) { quadSystem->ProcessEntity(quads[index], timeDelta); }
	};

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

	//enable the matrix uniforms
		void setMatrix()
		{
			//mesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>();
			mesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();

			//mesh->GetComponent<gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_modelMatrix>();
			mesh->GetComponent<gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_normalMatrix>();
		}
	};


//variables
	Scene					scene;			//the scene from the application
	MeshRenderSystem		meshSystem;		//the render systems
	ArcBallControlSystem	cameraControl;	//the camera controls
	PingPongRenderSystem*   pingPongSystem; //the ping pong render system


	Material globeMaterial,		//the sphere
		     splitMaterial,		//the highpass brightness filter	
			 bloomMaterial,		//the gaussian blur on the highpass
			 combineMaterial;	//combining the two textures

	Object* sphere; //the sphere



//after calling the constructor
	error_type Post_Initialize() override
	{
	//load the scene
		loadScene();

	//create a default material and set the light position
		globeMaterial   = createMaterial(globeVS,   globeFS);
		splitMaterial   = createMaterial(splitVS,   splitFS);
		bloomMaterial   = createMaterial(bloomVS,   bloomFS);
		combineMaterial = createMaterial(combineVS, combineFS);

	//set the light direction and color
		setLightProperties(math_t::Vec3f32(0.2f, 0.5f, 3.0f), math_t::Vec3f32(1.5f, 1.5f, 1.5f));

	//set the ping pong system's quads to the right shaders
		setQuadTextures();

	//connect the materials to the correct TO in the ping pong renderer
		connectPingPongSystemToMaterials();

	//initialize the sphere
		sphere = new Object(scene, meshSystem, "/models/globe.obj", globeMaterial);

		return Application::Post_Initialize();
	}

//load the scene
	void loadScene()
	{
		scene = GetScene();
			     	    scene->AddSystem<  gfx_cs::MaterialSystem      >();	//add material system
					    scene->AddSystem<  gfx_cs::CameraSystem        >();	//add camera
		meshSystem =    scene->AddSystem<  gfx_cs::MeshRenderSystem    >();	//add mesh render system	
					    scene->AddSystem<  gfx_cs::ArcBallSystem       >();	//add the arc ball system
		cameraControl = scene->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system


	//create rectangle that is the size of the screen
		math_t::Rectf_c screenSize(math_t::Rectf_c::width(GetWindow()->GetAspectRatio().Get()  * 2.0f),
								   math_t::Rectf_c::height(GetWindow()->GetAspectRatio().Get() * 2.0f));

	//set up the ping pong render system
		pingPongSystem = new PingPongRenderSystem(scene, scene->AddSystem<gfx_cs::MeshRenderSystem>(), screenSize);

	//set renderers
		meshSystem->SetRenderer(pingPongSystem->getFirstRenderObject()->GetRenderer());
		pingPongSystem->getQuadSystem()->SetRenderer(GetRenderer());					// TODO: move into struct

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

//set the shader's light properties
	void setLightProperties(math_t::Vec3f32 direction, math_t::Vec3f32 color)
	{
		gfx_gl::uniform_vso u_lightDirection; u_lightDirection->SetName("u_lightDirection").SetValueAs(direction);

		globeMaterial->GetShaderOperator()->AddUniform(*u_lightDirection);


		gfx_gl::uniform_vso u_lightColor; u_lightColor->SetName("u_lightColor").SetValueAs(color);

		globeMaterial->GetShaderOperator()->AddUniform(*u_lightColor);
	}

//set textures in shaders
	void setNormalColorTexture(Material material)
	{
		gfx_gl::uniform_vso u_normal; u_normal->SetName("texture_normal").SetValueAs(*pingPongSystem->getTO(0));

		material->GetShaderOperator()->AddUniform(*u_normal);
	}
	void setBrightColorTexture(Material material, TextureObject to)
	{
		gfx_gl::uniform_vso u_normal; u_normal->SetName("texture_bright").SetValueAs(*to);

		material->GetShaderOperator()->AddUniform(*u_normal);
	}

//set all of the textures for the quads
	void setQuadTextures()
	{
		setNormalColorTexture(splitMaterial);

		setBrightColorTexture(bloomMaterial, pingPongSystem->getTO(1));

		setNormalColorTexture(combineMaterial);
		setBrightColorTexture(combineMaterial, pingPongSystem->getTO(2));
	}

//connect the materials to the right quads in the pingpong render system
	void connectPingPongSystemToMaterials()
	{
		scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(pingPongSystem->getQuad(0), splitMaterial));
		scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(pingPongSystem->getQuad(1), bloomMaterial));
		scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(pingPongSystem->getQuad(2), combineMaterial));
	}

//render the scene
	void DoRender(sec_type timeDelta) override
	{
	//process the scene
		scene->Process(timeDelta);

	//render the scene as is
		pingPongSystem->setRenderer(0);
		pingPongSystem->render();
		
	//first pass: perform the high pass
		pingPongSystem->setRenderer(1);
		pingPongSystem->RenderEntity(0, timeDelta);
		pingPongSystem->render();
		scene->GetEntityManager()->DeactivateEntity(pingPongSystem->getQuad(0));	//no longer needed (don't render on the screen)

	//second pass: apply bloom to the high pass
		pingPongSystem->setRenderer(GetRenderer());
		pingPongSystem->RenderEntity(1, timeDelta);
		pingPongSystem->render();
		//scene->GetEntityManager()->DeactivateEntity(pingPongSystem->getQuad(1));	//no longer needed (don't render on the screen)

	//third pass: combine textures
		//pingPongSystem->setRenderer(GetRenderer());		//render to screen
		//pingPongSystem->RenderEntity(2, timeDelta);
		//pingPongSystem->render();
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
