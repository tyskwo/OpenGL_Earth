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
//globe vertex and fragment shader paths
	core_str::String globeShaderPathVS("/shaders/globeVS.glsl");
	core_str::String globeShaderPathFS("/shaders/globeFS.glsl");

//skybox vertex and fragment shader paths
	core_str::String skyboxShaderPathVS("/shaders/skyboxVS.glsl");
	core_str::String skyboxShaderPathFS("/shaders/skyboxFS.glsl");
};





/////////////////////////////////////////////////////////////////////////
// globe lab

class Program : public Application
{
public:
	Program() : Application("Earth") { }


private:

//typedefs
	typedef ecs_ptr																			SceneReference;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		RenderSystem;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		MeshRenderSystem;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		SkyBoxMeshRenderSystem;
	typedef gfx_rend::renderer_sptr															Renderer;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
	typedef core::smart_ptr::VirtualPtr<input::component_system::ArcBallControlSystem>		ArcBallControlSystem;
	typedef gfx_cs::material_sptr 															Material;


//struct for a scene and its components
	struct Scene
	{
	private:
		SceneReference			scene;					//reference to the scene
		RenderSystem			renderSystem;			//reference to the scene's render system
		Renderer				renderer;				//reference to the renderer
		ArcBallControlSystem	cameraControlSystem;	//the camera controls

	public:
	//intialize and create the object
		Scene(SceneReference sceneReference)
		{
			scene = sceneReference;

			scene->Initialize();

						   scene->AddSystem<gfx_cs::MaterialSystem>();		//add material system
			renderSystem = scene->AddSystem<gfx_cs::MeshRenderSystem>();	//add mesh render system
		}

	//getters
		SceneReference			GetScene()					{ return scene; }
		RenderSystem			GetRenderSystem()			{ return renderSystem; }
		Renderer				GetRenderer()				{ return renderer; }
		ArcBallControlSystem	GetCameraControlSystem()	{ return cameraControlSystem; }

	//initialize the camera
		void initializeCamera()
		{
								  scene->AddSystem<gfx_cs::CameraSystem>();				//add camera
								  scene->AddSystem<gfx_cs::ArcBallSystem>();			//add the arc ball system
			cameraControlSystem = scene->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system
		}

	//set the renderer
		void SetRenderer(Renderer rendererReference) { renderer = rendererReference; }
	};

//struct for a 3D object
	struct Object
	{
	private:
		Scene*				scene;			//reference to the scene
		core_str::String	objectPath;		//the path to the obj file
		Entity				mesh;			//the object's mesh
		Material			material;		//the object's material

	public:
	//intialize and create the object
		Object(Scene* sceneReference, core_str::String filePath, Material materialReference)
		{
			objectPath = filePath;
			scene = sceneReference;

			mesh = createMesh(objectPath);
			material = materialReference;

			scene->GetScene()->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(mesh, material));
		}

	//getters for the mesh and material
		Entity	 GetMesh()		{ return mesh; }
		Material GetMaterial()  { return material; }

	//setters for the mesh and material
		void SetMesh(Entity mesh)			    { this->mesh = mesh; }
		void SetMesh(core_str::String filePath) { this->mesh = createMesh(filePath); }
		void SetMaterial(Material material)     { this->material = material; }

	//get the path of the given string
		core_io::Path getPath(core_str::String objectPath)
		{
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
			return scene->GetScene()->CreatePrefab<pref_gfx::Mesh>().Create(loadObject(objectPath));
		}
	};




//variables
	Scene*		scene;			//scene from the application
	Scene*		skyBoxScene;	//scene for skybox

	Material	globeMaterial;	//the globe material
	Material	skyboxMaterial;	//the skybox material

	Object*		globe;			//the globe
	Object*		skybox;			//the 'space' box


//program specific variables
	float			earthAngle       =  0.0f;
	float			earthAngleDelta  =  0.001f;
	float			cloudAngle       =  0.0f;
	float			cloudAngleDelta  = -0.0001f;	//weather generally travels west->east.
	float			starTwinkleTime  =  0.0f;		//current time
	float			starTwinkleDelta =  0.005f;

	math_t::Vec3f32 lightPosition  = math_t::Vec3f32(-1, 0, 3); //-x so that the mountains cast correct shadows.
	math_t::Vec3f32 cameraPosition = math_t::Vec3f32( 0, 0, 2);





//after calling the constructor
	error_type Post_Initialize() override
	{
	//load the scene
		loadScenes();

	//create a default material and set the light position
		globeMaterial  = createMaterial(scene->GetScene(),        globeShaderPathVS,  globeShaderPathFS);
		skyboxMaterial = createMaterial(skyBoxScene->GetScene(), skyboxShaderPathVS, skyboxShaderPathFS);

	//add uniforms to the shaders
		addUniforms();

	//initialize the objects
		globe  = new Object(scene,       "/models/globe.obj",   globeMaterial);
		skybox = new Object(skyBoxScene, "/models/skybox.obj", skyboxMaterial);

		
		auto skyBoxMesh = skybox->GetMesh();
			
		skyBoxMesh->GetComponent<    gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_modelMatrix>(false);
		skyBoxMesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();
		skyBoxMesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_projectionMatrix>();
		skyBoxMesh->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>(false);


		return Application::Post_Initialize();
	}

//load the scene
	void loadScenes()
	{
		auto scn = GetScene();
		auto sky = core_sptr::MakeShared<core_cs::ECS>();
		sky->Initialize();
		scene		= new Scene(scn);
		skyBoxScene = new Scene(sky);

		scene->initializeCamera();


		gfx_rend::Renderer::Params skyboxRenderParams(GetRenderer()->GetParams());
		skyboxRenderParams.SetDepthWrite(false);

		gfx_rend::Renderer::Params renderParams = GetRenderer()->GetParams();
		renderParams.RemoveClearBit<gfx_rend::p_renderer::clear::ColorBufferBit>();
		GetRenderer()->SetParams(renderParams);

		
		



	//set renderer
		      scene->SetRenderer(GetRenderer());
		skyBoxScene->SetRenderer(core_sptr::MakeShared<gfx_rend::Renderer>(skyboxRenderParams));


	//create and set the camera
		auto camera = createCamera(true, 0.1f, 100.0f, 90.0f, cameraPosition);

		      scene->GetRenderSystem()->SetCamera(camera);
		skyBoxScene->GetRenderSystem()->SetCamera(camera);

		//set up the mouse and keyboard
		registerInputDevices();
	}

//create a camera
	entity_ptr createCamera(bool isPerspectiveView, float nearPlane, float farPlane, float verticalFOV_degrees, math_t::Vec3f32 position)
	{
		entity_ptr cameraEntity = scene->GetScene()->CreatePrefab<pref_gfx::Camera>()
			.Perspective(isPerspectiveView)
			.Near(nearPlane)
			.Far(farPlane)
			.VerticalFOV(math_t::Degree(verticalFOV_degrees))
			.Create(GetWindow()->GetDimensions());

	//add the camera to the arcball system
		scene->GetScene()->CreatePrefab<pref_gfx::ArcBall>().Add(cameraEntity);
		scene->GetScene()->CreatePrefab<pref_input::ArcBallControl>()
			.GlobalMultiplier(math_t::Vec2f(0.01f, 0.01f))
			.Add(cameraEntity);

	//change camera's position
		cameraEntity->GetComponent<math_cs::Transform>()->SetPosition(position);

		return cameraEntity;
	}

//create material
	Material createMaterial(SceneReference sceneReference, core_str::String vertexShader, core_str::String fragmentShader)
	{
		auto materialEntity = sceneReference->CreatePrefab<pref_gfx::Material>()
			.AssetsPath(GetAssetsPath())
			.Create(core_io::Path(vertexShader), core_io::Path(fragmentShader));

		return materialEntity->GetComponent<gfx_cs::Material>();
	}

//create the mouse and keyboard
	void registerInputDevices()
	{
		GetKeyboard()->Register(&*(scene->GetCameraControlSystem()));
		GetMouse()->Register(&*(scene->GetCameraControlSystem()));
	}

//add the uniforms to the shaders
	void addUniforms()
	{
		setLightPosition();
		setCloudRotation();
		setStarTwinkle();
		setTextures();
	}

//set the shader's light position
	void setLightPosition()
	{
		gfx_gl::uniform_vso u_lightPosition; u_lightPosition->SetName("u_lightPosition").SetValueAs(lightPosition);

		globeMaterial->GetShaderOperator()->AddUniform(*u_lightPosition);
	}

//set the shader's cloud rotation
	void setCloudRotation()
	{
		gfx_gl::uniform_vso u_cloudAngle; u_cloudAngle->SetName("u_cloudAngle").SetValueAs(cloudAngle);

		globeMaterial->GetShaderOperator()->AddUniform(*u_cloudAngle);
	}

//update the clouds rotation
	void updateCloudRotation()
	{
		gfx_gl::f_shader_operator::GetUniform(*globeMaterial->GetShaderOperator(), "u_cloudAngle")->SetValueAs(cloudAngle);
	}

//set the shader's twinkle
	void setStarTwinkle()
	{
		gfx_gl::uniform_vso u_twinkleTime; u_twinkleTime->SetName("u_twinkleTime").SetValueAs(starTwinkleTime);

		skyboxMaterial->GetShaderOperator()->AddUniform(*u_twinkleTime);
	}

//update the clouds rotation
	void updateStarTwinkle()
	{
		gfx_gl::f_shader_operator::GetUniform(*skyboxMaterial->GetShaderOperator(), "u_twinkleTime")->SetValueAs(starTwinkleTime);
	}

//set the globe's texture uniforms
	void setGlobeTextures()
	{
	//parameter to repeat wrap the cloud texture
		gfx_gl::TextureObject::Params cloudParams;
		cloudParams.Wrap_S<gfx_gl::p_texture_object::wrap_technique::Repeat>()
			.Wrap_T<gfx_gl::p_texture_object::wrap_technique::Repeat>()
			.Wrap_R<gfx_gl::p_texture_object::wrap_technique::Repeat>();

	//load the textures
		auto earthTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_diffuse.jpg")));
		auto earthNight = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_night.jpg")));
		auto earthClouds = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/clouds.jpg")), cloudParams);
		auto earthSpecular = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_specular.jpg")));
		auto earthNormal = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_normal_map.png")));

	//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("earth_diffuse").SetValueAs(*earthTexture);
		gfx_gl::uniform_vso specular; specular->SetName("earth_specular").SetValueAs(*earthSpecular);
		gfx_gl::uniform_vso night;    night->SetName("earth_night").SetValueAs(*earthNight);
		gfx_gl::uniform_vso clouds;   clouds->SetName("earth_clouds").SetValueAs(*earthClouds);
		gfx_gl::uniform_vso normals;  normals->SetName("earth_normals").SetValueAs(*earthNormal);

	//add to shader
		globeMaterial->GetShaderOperator()->AddUniform(*diffuse);
		globeMaterial->GetShaderOperator()->AddUniform(*specular);
		globeMaterial->GetShaderOperator()->AddUniform(*night);
		globeMaterial->GetShaderOperator()->AddUniform(*clouds);
		globeMaterial->GetShaderOperator()->AddUniform(*normals);
	}

//set the skybox texture uniform
	void setSkyboxTextures()
	{
	//load the texture
		auto skyboxTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/skybox.png")));

	//set the uniform
		gfx_gl::uniform_vso skybox; skybox->SetName("skybox_diffuse").SetValueAs(*skyboxTexture);

	//add to shader
		skyboxMaterial->GetShaderOperator()->AddUniform(*skybox);
	}

//set the texture uniforms in the shaders
	void setTextures()
	{
		setGlobeTextures();
		setSkyboxTextures();
	}

//render the skybox scene before we render the globe
	void Pre_Render(sec_type) override
	{
	//process the scene
		skyBoxScene->GetScene()->Process();

	//and render it
		skyBoxScene->GetRenderer()->ApplyRenderSettings();
		skyBoxScene->GetRenderer()->Render();
	}

//slowly rotate the earth
	void DoUpdate(sec_type delta) override
	{
	//apply rotation to globe's transform
		auto temp = globe->GetMesh()->GetComponent<math_cs::Transform>()->GetOrientation();
		temp.MakeRotationY(earthAngle);
		globe->GetMesh()->GetComponent<math_cs::Transform>()->SetOrientation(temp);

	//increase earth's rotation
		earthAngle += earthAngleDelta;

	//increase cloud's rotation
		cloudAngle += cloudAngleDelta;
		updateCloudRotation();

	//increase time for twinkle
		starTwinkleTime += starTwinkleDelta;
		updateStarTwinkle();

	//call Application's DoUpdate for camera controls
		Application::DoUpdate(delta);
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