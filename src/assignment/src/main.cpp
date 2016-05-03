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

	//moon vertex and fragment shader paths
	core_str::String moonShaderPathVS("/shaders/moonVS.glsl");
	core_str::String moonShaderPathFS("/shaders/moonFS.glsl");

	//skybox vertex and fragment shader paths
	core_str::String skyboxShaderPathVS("/shaders/skyboxVS.glsl");
	core_str::String skyboxShaderPathFS("/shaders/skyboxFS.glsl");

	core_str::String splitVS("/shaders/splitVS.glsl");
	core_str::String splitFS("/shaders/splitFS.glsl");

	core_str::String bloomVS("/shaders/bloomVS.glsl");
	core_str::String bloomFS("/shaders/bloomFS.glsl");

	core_str::String combineVS("/shaders/combineVS.glsl");
	core_str::String combineFS("/shaders/combineFS.glsl");

	//VS and FS for Billboard
	core_str::String shaderPathOneTextureBillboardVS("/shaders/tlocOneTextureBillboardVS.glsl");
	core_str::String shaderPathOneTextureBillboardFS("/shaders/tlocOneTexturePlusLightStencilFS.glsl");
};





/////////////////////////////////////////////////////////////////////////
// globe lab

class Program : public Application
{
public:
	Program() : Application("Earth") { }


private:

	//typedefs
	typedef ecs_ptr																			Scene;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		MeshRenderSystem;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		SkyBoxMeshRenderSystem;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
	typedef core::smart_ptr::VirtualPtr<input::component_system::ArcBallControlSystem>		ArcBallControlSystem;
	typedef gfx_cs::material_sptr 															Material;
	typedef core::smart_ptr::SharedPtr<tloc::graphics::gl::TextureObject>					TextureObject;
	typedef core::smart_ptr::SharedPtr<graphics::renderer::Renderer>						Renderer;

	//struct for a 3D object
	struct Object
	{
	private:
		Scene				scene;			//reference to the scene
		core_str::String	objectPath;		//the path to the obj file
		Entity				mesh;			//the object's mesh
		Material			material;		//the object's material

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
			return scene->CreatePrefab<pref_gfx::Mesh>().Create(loadObject(objectPath));
		}
	};


	//struct for a Billboard
	struct Billboard
	{
	private:
		Scene				scene;			//reference to the scene
		Entity				mesh;			//the object's mesh
		Material			material;		//the object's material

	public:
		//intialize and create the object
		Billboard(Scene sceneReference, Material materialReference)
		{
			scene = sceneReference;

			mesh = createMesh();
			material = materialReference;

			scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(mesh, material));
		}

		//getters for the mesh and material
		Entity	 GetMesh()		{ return mesh; }
		Material GetMaterial()  { return material; }

		//setters for the mesh and material
		void SetMesh(Entity mesh)			    { this->mesh = mesh; }
		void SetMaterial(Material material)     { this->material = material; }

		//create mesh from given object path
		Entity createMesh()
		{
			auto rect = math_t::Rectf32_c(math_t::Rectf32_c::width(1.5f), math_t::Rectf32_c::height(1.5f));
			return scene->CreatePrefab<pref_gfx::Quad>().Dimensions(rect).Create();
		}
	};

	//variables
	Scene					scene;			//the scene from the application
	Scene					skyBoxScene;	//scene for skybox
	MeshRenderSystem		meshSystem;		//the render system
	ArcBallControlSystem	cameraControl;	//the camera controls

	Material globeMaterial;		//the globe material
	Material moonMaterial;		//the moons material
	Material skyboxMaterial;	//the skybox material
	Material lightMaterial; 	//the light material

	Object*  globe;				//the globe
	Object*  moon;				//the globe
	Object*  skybox;			//the 'space' box
	Object*  sun;
	Billboard* light;

	//program specific variables
	float			earthAngle = 0.0f;
	float			m_accumAngle = 0.0f;
	float			earthAngleDelta = 0.001f;
	float			cloudAngle = 0.0f;
	float			cloudAngleDelta = -0.0001f;	//weather generally travels west->east.
	float			starTwinkleTime = 0.0f;		//current time
	float			starTwinkleDelta = 0.005f;

	math_t::Vec3f32 lightPosition = math_t::Vec3f32(-1, 0, 3); //-x so that the mountains cast correct shadows.
	math_t::Vec3f32 cameraPosition = math_t::Vec3f32(0, 0, 2);
	math_t::Vec3f32 moonPos = math_t::Vec3f32(1.6f, 0.0f, 0.0f);

	SkyBoxMeshRenderSystem	skyBoxMeshRenderSystem; //SkyBoxRenderSystem
	gfx_rend::renderer_sptr SkyBoxRenderer;


	//after calling the constructor
	error_type Post_Initialize() override
	{
		//load the scene
		loadScene();

		//create a default material and set the light position
		globeMaterial = createMaterial(scene, globeShaderPathVS, globeShaderPathFS);
		moonMaterial = createMaterial(scene, globeShaderPathVS, moonShaderPathFS);
		skyboxMaterial = createMaterial(skyBoxScene, skyboxShaderPathVS, skyboxShaderPathFS);
		lightMaterial = createMaterial(scene, shaderPathOneTextureBillboardVS, shaderPathOneTextureBillboardFS);

		//add uniforms to the shaders
		addUniforms();


		//---------------------------------------------------------------------------------

		lightMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>(false);
		lightMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();
		lightMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_projectionMatrix>();


		//---------------------------------------------------------------------------------


		//initialize the objects
		globe = new Object(scene, "/models/globe.obj", globeMaterial);
		moon = new Object(scene, "/models/globe.obj", moonMaterial);

		//---------------------------------------------------------------------------------
		light = new Billboard(scene, lightMaterial);
		light->GetMesh()->GetComponent<math_cs::Transform>()->SetPosition(lightPosition);
		//---------------------------------------------------------------------------------


		auto moonTransform = moon->GetMesh()->GetComponent<math_cs::Transform>();
		moonTransform->SetScale(moonTransform->GetScale() / 6.0f);
		moonTransform->SetPosition(moonPos);

		skybox = new Object(skyBoxScene, "/models/skybox.obj", skyboxMaterial);

		
		auto skyBoxMesh = skybox->GetMesh();
			
		skyBoxMesh->GetComponent<gfx_cs::Mesh>()->
			SetEnableUniform<gfx_cs::p_renderable::uniforms::k_modelMatrix>(false);
		skyBoxMesh->GetComponent<gfx_cs::Material>()->
			SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();
		skyBoxMesh->GetComponent<gfx_cs::Material>()->
			SetEnableUniform<gfx_cs::p_material::uniforms::k_projectionMatrix>();
		skyBoxMesh->GetComponent<gfx_cs::Material>()->
			SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>(false);
		

		skyBoxScene->Initialize();

		return Application::Post_Initialize();
	}

	//load the scene
	void loadScene()
	{
		scene = GetScene();
		scene->AddSystem<gfx_cs::MaterialSystem>();							//add material system
		scene->AddSystem<gfx_cs::CameraSystem>();							//add camera
		meshSystem = scene->AddSystem<gfx_cs::MeshRenderSystem>();			//add mesh render system	
		scene->AddSystem<gfx_cs::ArcBallSystem>();							//add the arc ball system
		cameraControl = scene->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system

		gfx_rend::Renderer::Params skyboxRenderParams(GetRenderer()->GetParams());
		skyboxRenderParams.SetDepthWrite(false);
		SkyBoxRenderer = core_sptr::MakeShared<gfx_rend::Renderer>(skyboxRenderParams);

		auto renderParams = GetRenderer()->GetParams();
		renderParams.RemoveClearBit<gfx_rend::p_renderer::clear::ColorBufferBit>();
		GetRenderer()->SetParams(renderParams);

		skyBoxScene = core_sptr::MakeShared<core_cs::ECS>();
		skyBoxScene->AddSystem<gfx_cs::MaterialSystem>();							//add material system
		skyBoxMeshRenderSystem = skyBoxScene->AddSystem<gfx_cs::MeshRenderSystem>();	//add mesh render system
		skyBoxMeshRenderSystem->SetRenderer(SkyBoxRenderer);

		//set renderer
		meshSystem->SetRenderer(GetRenderer());
		skyBoxMeshRenderSystem->SetRenderer(SkyBoxRenderer);

		//set the background color
		gfx_rend::Renderer::Params clearColor(GetRenderer()->GetParams());
		clearColor.SetClearColor(gfx_t::Color(0.06f, 0.06f, 0.08f, 1.0f));
		GetRenderer()->SetParams(clearColor);

		auto camera = createCamera(true, 0.1f, 100.0f, 90.0f, cameraPosition);

		//create and set the camera
		meshSystem->SetCamera(camera);
		skyBoxMeshRenderSystem->SetCamera(camera);

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
			
			scene->CreatePrefab<pref_gfx::ArcBall>().Add(cameraEntity);
		scene->CreatePrefab<pref_input::ArcBallControl>()
			.GlobalMultiplier(math_t::Vec2f(0.01f, 0.01f))
			.Add(cameraEntity);

		//change camera's position
		cameraEntity->GetComponent<math_cs::Transform>()->SetPosition(position);

		return cameraEntity;
	}

	//create material
	Material createMaterial(Scene sceneReference, core_str::String vertexShader, core_str::String fragmentShader)
	{
		auto materialEntity = sceneReference->CreatePrefab<pref_gfx::Material>()
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
		moonMaterial->GetShaderOperator()->AddUniform(*u_lightPosition);
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
		auto waterNormal = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/waterNormals.jpg")), cloudParams);

		//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("earth_diffuse").SetValueAs(*earthTexture);
		gfx_gl::uniform_vso specular; specular->SetName("earth_specular").SetValueAs(*earthSpecular);
		gfx_gl::uniform_vso night;    night->SetName("earth_night").SetValueAs(*earthNight);
		gfx_gl::uniform_vso clouds;   clouds->SetName("earth_clouds").SetValueAs(*earthClouds);
		gfx_gl::uniform_vso normals;  normals->SetName("earth_normals").SetValueAs(*earthNormal);
		gfx_gl::uniform_vso waterNormals;  waterNormals->SetName("water_normals").SetValueAs(*waterNormal);

		//add to shader
		globeMaterial->GetShaderOperator()->AddUniform(*diffuse);
		globeMaterial->GetShaderOperator()->AddUniform(*specular);
		globeMaterial->GetShaderOperator()->AddUniform(*night);
		globeMaterial->GetShaderOperator()->AddUniform(*clouds);
		globeMaterial->GetShaderOperator()->AddUniform(*normals);
		globeMaterial->GetShaderOperator()->AddUniform(*waterNormals);
	}

	//set the globe's texture uniforms
	void setMoonTextures()
	{
		//load the textures
		auto moonTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/moon_diffuse.png")));
		auto moonNormal = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/moon_normals.jpg")));

		//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("moon_diffuse").SetValueAs(*moonTexture);
		gfx_gl::uniform_vso normals;  normals->SetName("moon_normals").SetValueAs(*moonNormal);

		//add to shader
		moonMaterial->GetShaderOperator()->AddUniform(*diffuse);
		moonMaterial->GetShaderOperator()->AddUniform(*normals);
	}

	//set the globe's texture uniforms
	void setLightTextures()
	{
		//load the textures
		auto lightTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/light.png")));

		//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("s_texture").SetValueAs(*lightTexture);

		//add to shader
		lightMaterial->GetShaderOperator()->AddUniform(*diffuse);
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
		setMoonTextures();
		setGlobeTextures();
		setSkyboxTextures();
		setLightTextures();
	}

	void Pre_Render(sec_type delta) override
	{
		skyBoxScene->Process(delta);

		SkyBoxRenderer->ApplyRenderSettings();
		SkyBoxRenderer->Render();
	}

	//slowly rotate the earth
	void DoUpdate(sec_type delta) override
	{
		m_accumAngle += float(delta) * 50.0f;

		auto xPos = math::Sin(math_t::Radian(math_t::Degree(core_utils::CastNumber<f32>(m_accumAngle)))) * 1.5f;
		auto zPos = math::Cos(math_t::Radian(math_t::Degree(core_utils::CastNumber<f32>(m_accumAngle)))) * 1.5f;

		moonPos[0] = xPos;
		moonPos[2] = zPos;



		//apply rotation to globe's transform
		auto temp = globe->GetMesh()->GetComponent<math_cs::Transform>()->GetOrientation();
		temp.MakeRotationY(earthAngle);
		globe->GetMesh()->GetComponent<math_cs::Transform>()->SetOrientation(temp);

		//apply rotation to moon's transform
		auto tempMoon = moon->GetMesh()->GetComponent<math_cs::Transform>()->GetOrientation();
		tempMoon.MakeRotationY(earthAngle);
		moon->GetMesh()->GetComponent<math_cs::Transform>()->SetOrientation(tempMoon);
		moon->GetMesh()->GetComponent<math_cs::Transform>()->SetPosition(moonPos);

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