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
	core_str::String globeVS("/shaders/globeVS.glsl");
	core_str::String globeFS("/shaders/globeFS.glsl");

//moon vertex and fragment shader paths
	core_str::String moonVS("/shaders/moonVS.glsl");
	core_str::String moonFS("/shaders/moonFS.glsl");

//skybox vertex and fragment shader paths
	core_str::String skyboxVS("/shaders/skyboxVS.glsl");
	core_str::String skyboxFS("/shaders/skyboxFS.glsl");

//billboard vertex and fragment shader paths
	core_str::String billboardVS("/shaders/billboardVS.glsl");
	core_str::String billboardFS("/shaders/tlocOneTexturePlusLightStencilFS.glsl");

	core_str::String oneTextureVS("/shaders/tlocOneTextureVS.glsl");
	core_str::String oneTextureFS("/shaders/tlocOneTextureFS.glsl");

	core_str::String splitVS("/shaders/splitVS.glsl");
	core_str::String splitFS("/shaders/splitFS.glsl");

	core_str::String bloomVS("/shaders/tlocBloomVS.glsl");
	core_str::String bloomFS("/shaders/tlocBloomFS.glsl");

	core_str::String combineVS("/shaders/combineVS.glsl");
	core_str::String combineFS("/shaders/combineFS.glsl");

	core_str::String shaderPathGodrayVS("/shaders/tlocPostProcessGodrayVS.glsl");
	core_str::String shaderPathGodrayFS("/shaders/tlocPostProcessGodrayFS.glsl");

	core_str::String shaderPathAdditiveVS("/shaders/tlocOneTextureVS.glsl");
	core_str::String shaderPathAdditiveFS("/shaders/tlocAdditiveBlendingFS.glsl");

	core_str::String shaderPathGaussianBlurVS("/shaders/tlocGaussianBlurVS.glsl");
	core_str::String shaderPathGaussianBlurFS("/shaders/tlocGaussianBlurFS.glsl");
};





/////////////////////////////////////////////////////////////////////////
// final project

class Program : public Application
{
public:
	Program() : Application("Earth") { }


private:

//typedefs
	typedef ecs_ptr																			ECS;
	typedef core::smart_ptr::VirtualPtr<graphics::component_system::MeshRenderSystem>		RenderSystem;
	typedef core::smart_ptr::VirtualPtr<core::component_system::Entity>						Entity;
	typedef core::smart_ptr::VirtualPtr<input::component_system::ArcBallControlSystem>		ArcBallControlSystem;
	typedef gfx_cs::material_sptr 															Material;
	typedef core::smart_ptr::SharedPtr<tloc::graphics::gl::TextureObject>					TextureObject;
	typedef gfx_rend::renderer_sptr															Renderer;






//struct for a scene
	struct Scene
	{
		ECS				ecs;
		RenderSystem	renderSystem;
		Renderer		renderer;

		Scene()
		{
			ecs = core_sptr::MakeShared<core_cs::ECS>();

			ecs->AddSystem<gfx_cs::MaterialSystem>();
			renderSystem = ecs->AddSystem<gfx_cs::MeshRenderSystem>();
		}

		Scene(ECS ecsReference)
		{
			ecs = ecsReference;

			ecs->AddSystem<gfx_cs::MaterialSystem>();
			renderSystem = ecs->AddSystem<gfx_cs::MeshRenderSystem>();
		}

		void SetRenderer(gfx_rend::Renderer::Params rendererParams)
		{ 
			renderer = core_sptr::MakeShared<gfx_rend::Renderer>(rendererParams);
			renderSystem->SetRenderer(renderer);
		}

		void SetRenderer(Renderer rendererReference)
		{
			renderer = rendererReference;
			renderSystem->SetRenderer(renderer);
		}

		ArcBallControlSystem AddCamera()
		{
					ecs->AddSystem<gfx_cs::CameraSystem>();				//add camera
					ecs->AddSystem<gfx_cs::ArcBallSystem>();			//add the arc ball system
			return	ecs->AddSystem<input_cs::ArcBallControlSystem>();	//add the control system
		}
	};

//struct for a 3D object
	struct Object
	{
	private:
		ECS					scene;			//reference to the scene
		RenderSystem		renderSystem;	//reference to the scene's render system
		core_str::String	objectPath;		//the path to the obj file
		Entity				mesh;			//the object's mesh
		Material			material;		//the object's material

	public:
	//intialize and create the object
		Object(Scene* sceneReference, core_str::String filePath, Material materialReference)
		{
			objectPath	 = filePath;
			scene		 = sceneReference->ecs;
			renderSystem = sceneReference->renderSystem;

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
		void SetPosition(math_t::Vec3f32 pos)	{ mesh->GetComponent<math_cs::Transform>()->SetPosition(pos); }
		void SetScale(float scale) 
		{
			auto moonTransform = mesh->GetComponent<math_cs::Transform>();
			moonTransform->SetScale(moonTransform->GetScale() / scale);
		}

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
			return scene->CreatePrefab<pref_gfx::Mesh>().DispatchTo(renderSystem.get()).Create(loadObject(objectPath));
		}
	};

//struct for a Billboard
	struct Billboard
	{
	private:
		ECS					scene;			//reference to the scene
		RenderSystem		renderSystem;	//reference to the scene's render system
		Entity				quad;			//the billboard's quad
		Material			material;		//the billboard's material

	public:
	//intialize and create the billboard
		Billboard(Scene* sceneReference, Material materialReference)
		{
			scene		 = sceneReference->ecs;
			renderSystem = sceneReference->renderSystem;

			quad = createQuad();
			material = materialReference;

			scene->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(quad, material));
		}

	//getters for the quad and material
		Entity	 GetQuad()		{ return quad; }
		Material GetMaterial()  { return material; }

	//setters for the quad and material
		void SetQuad(Entity quad)			    { this->quad = quad; }
		void SetMaterial(Material material)     { this->material = material; }
		void SetPosition(math_t::Vec3f32 pos)	{ quad->GetComponent<math_cs::Transform>()->SetPosition(pos); }

	//create quad
		Entity createQuad()
		{
			auto rect = math_t::Rectf32_c(math_t::Rectf32_c::width(1.5f), math_t::Rectf32_c::height(1.5f));
			return scene->CreatePrefab<pref_gfx::Quad>().Dimensions(rect).DispatchTo(renderSystem.get()).Create();
		}
	};




	


//variables
	Scene*		scn_main;			//the scenes
	Scene*		scn_skybox;
	Scene*		scn_sun;
	Scene*		scn_rtt;
	Scene*		scn_BlurHor;
	Scene*		scn_BlurVert;



	Entity					camera;
	ArcBallControlSystem	cameraControl;		//the camera controls




	Material globeMaterial;		//the globe material
	Material moonMaterial;		//the moons material
	Material skyboxMaterial;	//the skybox material
	Material sunMaterial; 		//the light material
	Material rttMaterial;
	Material rttHorBlurMaterial;
	Material rttVertBlurMaterial;


	Object*		globe;			//the globe
	Object*		moon;			//the globe
	Object*		skybox;			//the 'space' box
	Billboard*	sun;			//the sun as a billboard




	gfx::Rtt*		rtt;
	gfx::Rtt*		rttBlurHor;
	gfx::Rtt*		rttBlurVert;
	TextureObject	rttTo;
	TextureObject   brightTo;
	TextureObject   rttHorTo;
	TextureObject   rttVertTo;
	Entity			rttQuad;
	Entity			rttHorQuad;
	Entity			rttVertQuad;




//program specific variables

//rotation variables
	float			earthAngle			= 0.0f;
	float			earthAngleDelta		= 0.001f;
	float			moonAngle			= 0.0f;
	float			moonAngleDelta		= 50.0f;
	float			moonDistance		= 1.5f;
	float			cloudAngle			= 0.0f;
	float			cloudAngleDelta		= -0.0001f;	//weather generally travels west->east.

//skybox variables
	float			starTwinkleTime		= 0.0f;		//current time
	float			starTwinkleDelta	= 0.005f;

//bloom variables
	float			bloomExposure		= 1.1f;
	int				blurPasses			= 8;
	float			shininess_earth		= 150.0f;	//lower = more shiny
	float			intensity_earth		= 1.1f;
	float			shininess_moon		= 2.0f;
	float			intensity_moon		= 2.2f;
	math_t::Vec3f32 sunIntensity		= math_t::Vec3f32(1.8f);


//position variables
	math_t::Vec3f32 sunPosition			= math_t::Vec3f32(  -2,    0,    6); //-x so that the mountains cast correct shadows.
	math_t::Vec3f32 cameraPosition		= math_t::Vec3f32(   0,    0,    2);
	math_t::Vec3f32 moonPosition		= math_t::Vec3f32(1.6f, 0.0f, 0.0f);

//debug variables
	bool cloudFlag = false;	//flag to draw the clouds









//after calling the constructor
	error_type Post_Initialize() override
	{
	//load the scene
		loadScenes();


	//create all necessary materials
		createMaterials();


	//add uniforms to the shaders
		addUniforms();


	//initialize the objects
		globe = new Object(scn_main, "/models/globe.obj", globeMaterial);

		moon = new Object(scn_main, "/models/globe.obj", moonMaterial);
		moon->SetPosition(moonPosition);
		moon->SetScale(6.0f);

		sun = new Billboard(scn_sun, sunMaterial);
		sun->SetPosition(sunPosition);

		skybox = new Object(scn_skybox, "/models/skybox.obj", skyboxMaterial);


	//set the matrix values
	//TO DO: update description
		setMatrices();	
	

	//initialize the rtt quad
		math_t::Rectf32_c rect2(math_t::Rectf32_c::width(2.0f), math_t::Rectf32_c::height(2.0f));
		math_t::Rectf32_c rect(math_t::Rectf32_c::width(2.0f), math_t::Rectf32_c::height(2.0f));

		rttQuad = scn_rtt->ecs->CreatePrefab<pref_gfx::Quad>().Dimensions(rect2).DispatchTo(scn_rtt->renderSystem.get()).Create();
		rttHorQuad = scn_BlurHor->ecs->CreatePrefab<pref_gfx::Quad>().Dimensions(rect).DispatchTo(scn_BlurHor->renderSystem.get()).Create();
		rttVertQuad = scn_BlurVert->ecs->CreatePrefab<pref_gfx::Quad>().Dimensions(rect).DispatchTo(scn_BlurVert->renderSystem.get()).Create();



		scn_rtt->ecs->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(rttQuad, rttMaterial));
		scn_BlurHor->ecs->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(rttHorQuad, rttHorBlurMaterial));
		scn_BlurVert->ecs->GetEntityManager()->InsertComponent(core_cs::EntityManager::Params(rttVertQuad, rttVertBlurMaterial));


	//initialize the skybox scene
		scn_skybox->ecs->Initialize();
		scn_sun->ecs->Initialize();
		scn_rtt->ecs->Initialize();
		scn_BlurHor->ecs->Initialize();
		scn_BlurVert->ecs->Initialize();


		return Application::Post_Initialize();
	}

//load the scene
	void loadScenes()
	{
		scn_main	 = new Scene(GetScene());
		scn_skybox	 = new Scene();
		scn_sun		 = new Scene();
		scn_rtt	     = new Scene();
		scn_BlurHor  = new Scene();
		scn_BlurVert = new Scene();

		cameraControl = scn_main->AddCamera();

	//initialize rtt
		rtt = new gfx::Rtt(core_ds::MakeTuple(800, 600));
		rttTo = rtt->AddColorAttachment<0, gfx_t::color_u16_rgba>();
		brightTo = rtt->AddColorAttachment<1, gfx_t::color_u16_rgba>();
		rtt->AddDepthAttachment();

		rttBlurHor = new gfx::Rtt(core_ds::MakeTuple(800, 600));
		rttHorTo = rttBlurHor->AddColorAttachment<0, gfx_t::color_u16_rgba>();
		rttBlurHor->AddDepthAttachment();

		rttBlurVert = new gfx::Rtt(core_ds::MakeTuple(800, 600));
		rttVertTo = rttBlurVert->AddColorAttachment<0>(brightTo);
		rttBlurVert->AddDepthAttachment();

	//create the renderers for each scene
		createRenderers();

	//create and set the camera
		camera = createCamera(true, 0.1f, 100.0f, 90.0f, cameraPosition);

		scn_main->renderSystem->SetCamera(camera);
		scn_skybox->renderSystem->SetCamera(camera);
		scn_sun->renderSystem->SetCamera(camera);

	//set up the mouse and keyboard
		registerInputDevices();
	}

//create the renderers for each screen
	void createRenderers()
	{
	//set up skybox renderer
		gfx_rend::Renderer::Params skyboxRenderParams(rtt->GetRenderer()->GetParams());
		skyboxRenderParams
			.SetDepthWrite(false);
		auto skyboxRenderer = core_sptr::MakeShared<gfx_rend::Renderer>(skyboxRenderParams);

	//set up scene_main renderer
		gfx_rend::Renderer::Params rttRenderParams(rtt->GetRenderer()->GetParams());
		rttRenderParams
			.RemoveClearBit<gfx_rend::p_renderer::clear::ColorBufferBit>()
			.Enable<gfx_rend::p_renderer::enable_disable::Blend>()
			.SetBlendFunction<gfx_rend::p_renderer::blend_function::SourceAlpha, gfx_rend::p_renderer::blend_function::OneMinusSourceAlpha>();
		auto rttRenderer = core_sptr::MakeShared<gfx_rend::Renderer>(rttRenderParams);

	//set up sun renderer is the same as the rtt renderer
		auto sunRenderer = rttRenderer;

		//rttBlurHor->GetRenderer()->SetParams(rttRenderParams);
		//rttBlurVert->GetRenderer()->SetParams(rttRenderParams);

		

	//set renderers
		scn_main->SetRenderer(rttRenderer);
		scn_skybox->SetRenderer(skyboxRenderer);
		scn_sun->SetRenderer(sunRenderer);
		scn_rtt->SetRenderer(GetRenderer());
		scn_BlurHor->SetRenderer(rttBlurHor->GetRenderer());
		scn_BlurVert->SetRenderer(rttBlurVert->GetRenderer());
	}

//create all the materials for the program
	void createMaterials()
	{
		globeMaterial	= createMaterial(scn_main->ecs, globeVS, globeFS);
		moonMaterial	= createMaterial(scn_main->ecs, globeVS, moonFS);
		skyboxMaterial	= createMaterial(scn_skybox->ecs, skyboxVS, skyboxFS);
		sunMaterial		= createMaterial(scn_main->ecs, billboardVS, billboardFS);
		rttHorBlurMaterial = createMaterial(scn_BlurHor->ecs, shaderPathGaussianBlurVS, shaderPathGaussianBlurFS);
		rttVertBlurMaterial = createMaterial(scn_BlurVert->ecs, shaderPathGaussianBlurVS, shaderPathGaussianBlurFS);
		//rttMaterial		= createMaterial(scn_rtt->ecs, oneTextureVS, oneTextureFS);
		rttMaterial = createMaterial(scn_rtt->ecs, bloomVS, bloomFS);
	}

//create a camera
	entity_ptr createCamera(bool isPerspectiveView, float nearPlane, float farPlane, float verticalFOV_degrees, math_t::Vec3f32 position)
	{
		entity_ptr cameraEntity = scn_main->ecs->CreatePrefab<pref_gfx::Camera>()
			.Perspective(isPerspectiveView)
			.Near(nearPlane)
			.Far(farPlane)
			.VerticalFOV(math_t::Degree(verticalFOV_degrees))
			.Create(GetWindow()->GetDimensions());

		scn_main->ecs->CreatePrefab<pref_gfx::ArcBall>().Add(cameraEntity);
		scn_main->ecs->CreatePrefab<pref_input::ArcBallControl>()
			.GlobalMultiplier(math_t::Vec2f(0.01f, 0.01f))
			.Add(cameraEntity);

	//change camera's position
		cameraEntity->GetComponent<math_cs::Transform>()->SetPosition(position);

		return cameraEntity;
	}

//create material
	Material createMaterial(ECS sceneReference, core_str::String vertexShader, core_str::String fragmentShader)
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
		setCloudFlag();
		setStarTwinkle();
		setTextures();
		setBloomParameters();
	}

//set the matrices of the skybox and light material
	void setMatrices()
	{
		skybox->GetMesh()->GetComponent<gfx_cs::Mesh>()->SetEnableUniform<gfx_cs::p_renderable::uniforms::k_modelMatrix>(false);
		skybox->GetMesh()->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();
		skybox->GetMesh()->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_projectionMatrix>();
		skybox->GetMesh()->GetComponent<gfx_cs::Material>()->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>(false);

		sunMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewProjectionMatrix>(false);
		sunMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_viewMatrix>();
		sunMaterial->SetEnableUniform<gfx_cs::p_material::uniforms::k_projectionMatrix>();
	}

//set the shader's light position
	void setLightPosition()
	{
		gfx_gl::uniform_vso u_lightPosition; u_lightPosition->SetName("u_lightPosition").SetValueAs(sunPosition);
		gfx_gl::uniform_vso u_lightColor; u_lightColor->SetName("u_lightColor").SetValueAs(sunIntensity);

		globeMaterial->GetShaderOperator()->AddUniform(*u_lightPosition);
		moonMaterial->GetShaderOperator()->AddUniform(*u_lightPosition);

		globeMaterial->GetShaderOperator()->AddUniform(*u_lightColor);
		moonMaterial->GetShaderOperator()->AddUniform(*u_lightColor);
		sunMaterial->GetShaderOperator()->AddUniform(*u_lightColor);
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

//set the cloud flag uniform
	void setCloudFlag()
	{
		gfx_gl::uniform_vso u_cloudFlag; u_cloudFlag->SetName("cloud_flag").SetValueAs(int(cloudFlag));

		globeMaterial->GetShaderOperator()->AddUniform(*u_cloudFlag);
	}

//update the clouds rotation
	void updateCloudFlag()
	{
		gfx_gl::f_shader_operator::GetUniform(*globeMaterial->GetShaderOperator(), "cloud_flag")->SetValueAs(int(cloudFlag));
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
		auto earthTexture  = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_diffuse.jpg")));
		auto earthNight    = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_night.jpg")));
		auto earthClouds   = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/clouds.jpg")), cloudParams);
		auto earthSpecular = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_specular.jpg")));
		auto earthNormal   = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/earth_normal_map.png")));
		auto waterNormal   = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/waterNormals.jpg")), cloudParams);

	//set the uniforms
		gfx_gl::uniform_vso diffuse;		diffuse->SetName("earth_diffuse").SetValueAs(*earthTexture);
		gfx_gl::uniform_vso specular;		specular->SetName("earth_specular").SetValueAs(*earthSpecular);
		gfx_gl::uniform_vso night;			night->SetName("earth_night").SetValueAs(*earthNight);
		gfx_gl::uniform_vso clouds;			clouds->SetName("earth_clouds").SetValueAs(*earthClouds);
		gfx_gl::uniform_vso normals;		normals->SetName("earth_normals").SetValueAs(*earthNormal);
		gfx_gl::uniform_vso waterNormals;	waterNormals->SetName("water_normals").SetValueAs(*waterNormal);

	//add to shader
		globeMaterial->GetShaderOperator()->AddUniform(*diffuse);
		globeMaterial->GetShaderOperator()->AddUniform(*specular);
		globeMaterial->GetShaderOperator()->AddUniform(*night);
		globeMaterial->GetShaderOperator()->AddUniform(*clouds);
		globeMaterial->GetShaderOperator()->AddUniform(*normals);
		globeMaterial->GetShaderOperator()->AddUniform(*waterNormals);
	}

//set the moon's texture uniforms
	void setMoonTextures()
	{
	//load the textures
		auto moonTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/moon_diffuse.png")));
		auto moonNormal  = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/moon_normals.jpg")));

	//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("moon_diffuse").SetValueAs(*moonTexture);
		gfx_gl::uniform_vso normals;  normals->SetName("moon_normals").SetValueAs(*moonNormal);

	//add to shader
		moonMaterial->GetShaderOperator()->AddUniform(*diffuse);
		moonMaterial->GetShaderOperator()->AddUniform(*normals);
	}

//set the sun's texture uniforms
	void setSunTextures()
	{
	//load the textures
		auto lightTexture = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(GetAssetsPath() + core_str::String("/images/light.png")));

	//set the uniforms
		gfx_gl::uniform_vso diffuse;  diffuse->SetName("s_texture").SetValueAs(*lightTexture);

	//add to shader
		sunMaterial->GetShaderOperator()->AddUniform(*diffuse);
	}

//set the rtt's texture uniforms
	void setRttTextures()
	{
	//set the uniforms
		//gfx_gl::uniform_vso diffuse;  diffuse->SetName("s_texture").SetValueAs(*rttTo);

		gfx_gl::uniform_vso u_rttColTo; u_rttColTo->SetName("s_texture").SetValueAs(*rttTo);

		gfx_gl::uniform_vso u_rttBrightTo; u_rttBrightTo->SetName("s_bright").SetValueAs(*brightTo);

		gfx_gl::uniform_vso u_exposure; u_exposure->SetName("u_exposure").SetValueAs(bloomExposure);



		//add to shader
		rttMaterial->GetShaderOperator()->AddUniform(*u_rttColTo);
		rttMaterial->GetShaderOperator()->AddUniform(*u_rttBrightTo);
		rttMaterial->GetShaderOperator()->AddUniform(*u_exposure);
	}

//set the blurr hor texture uniforms
	void setHorBlurTextures()
	{
		gfx_gl::uniform_vso u_rttBrightTo; u_rttBrightTo->SetName("s_texture").SetValueAs(*brightTo);

		gfx_gl::uniform_vso u_horizontal; u_horizontal->SetName("u_horizontal").SetValueAs(1);

		rttHorBlurMaterial->GetShaderOperator()->AddUniform(*u_rttBrightTo);
		rttHorBlurMaterial->GetShaderOperator()->AddUniform(*u_horizontal);
	}

//set the blurr vert texture uniforms
	void setVertBlurTextures()
	{
		gfx_gl::uniform_vso u_rttBrightTo; u_rttBrightTo->SetName("s_texture").SetValueAs(*rttHorTo);

		gfx_gl::uniform_vso u_horizontal; u_horizontal->SetName("u_horizontal").SetValueAs(0);

		rttVertBlurMaterial->GetShaderOperator()->AddUniform(*u_rttBrightTo);
		rttVertBlurMaterial->GetShaderOperator()->AddUniform(*u_horizontal);
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
		setSunTextures();
		setHorBlurTextures();
		setVertBlurTextures();
		setRttTextures();
	}

//set the paramters for bloom for easy tweaking
	void setBloomParameters()
	{
		gfx_gl::uniform_vso u_shininessEarth; u_shininessEarth->SetName("shininess").SetValueAs(shininess_earth);
		gfx_gl::uniform_vso u_shininessMoon;  u_shininessMoon->SetName("shininess").SetValueAs(shininess_moon);
		gfx_gl::uniform_vso u_intensityEarth; u_intensityEarth->SetName("specularIntensity").SetValueAs(intensity_earth);
		gfx_gl::uniform_vso u_intensityMoon;  u_intensityMoon->SetName("specularIntensity").SetValueAs(intensity_moon);




		globeMaterial->GetShaderOperator()->AddUniform(*u_shininessEarth);
		 moonMaterial->GetShaderOperator()->AddUniform(*u_shininessMoon);
		globeMaterial->GetShaderOperator()->AddUniform(*u_intensityEarth);
		 moonMaterial->GetShaderOperator()->AddUniform(*u_intensityMoon);
	}

	void DoRender(sec_type delta) override
	{
	//process the scenes
		scn_skybox->ecs->Process(delta);
		scn_sun->ecs->Process(delta);
		scn_main->ecs->Process(delta);

	//render the scenes
		scn_skybox->renderer->ApplyRenderSettings();
		scn_skybox->renderer->Render();

		scn_sun->renderer->ApplyRenderSettings();
		scn_sun->renderer->Render();

		scn_main->renderer->ApplyRenderSettings();
		scn_main->renderer->Render();

		for (int i = 0; i < blurPasses; ++i)
		{
			scn_BlurHor->ecs->Update(delta);
			scn_BlurHor->ecs->Process(delta);

			scn_BlurHor->renderer->ApplyRenderSettings();
			scn_BlurHor->renderer->Render();

			scn_BlurVert->ecs->Update(delta);
			scn_BlurVert->ecs->Process(delta);

			scn_BlurVert->renderer->ApplyRenderSettings();
			scn_BlurVert->renderer->Render();
		}

		scn_rtt->ecs->Update(delta);
		scn_rtt->ecs->Process(delta);
		scn_rtt->renderer->ApplyRenderSettings();
		scn_rtt->renderer->Render();
	}

	void DoUpdate(sec_type delta) override
	{
	//update the position of the moon
		moonAngle += float(delta) * moonAngleDelta;

	//set the x and z position
		moonPosition[0] = math::Sin(math_t::Radian(math_t::Degree(core_utils::CastNumber<f32>(moonAngle)))) * moonDistance;
		moonPosition[2] = math::Cos(math_t::Radian(math_t::Degree(core_utils::CastNumber<f32>(moonAngle)))) * moonDistance;


	//apply rotation to globe's transform
		auto temp = globe->GetMesh()->GetComponent<math_cs::Transform>()->GetOrientation();
		temp.MakeRotationY(earthAngle);
		globe->GetMesh()->GetComponent<math_cs::Transform>()->SetOrientation(temp);


	//apply rotation to moon's transform
		auto tempMoon = moon->GetMesh()->GetComponent<math_cs::Transform>()->GetOrientation();
		tempMoon.MakeRotationY(earthAngle);
		moon->GetMesh()->GetComponent<math_cs::Transform>()->SetOrientation(tempMoon);
		moon->GetMesh()->GetComponent<math_cs::Transform>()->SetPosition(moonPosition);


	//increase earth's rotation
		earthAngle += earthAngleDelta;


	//increase cloud's rotation
		cloudAngle += cloudAngleDelta;
		updateCloudRotation();


	//increase time for twinkle
		starTwinkleTime += starTwinkleDelta;
		updateStarTwinkle();


	//focus the camera back on the mesh
		if (GetKeyboard()->IsKeyDown(input_hid::KeyboardEvent::f))
		{ camera->GetComponent<gfx_cs::ArcBall>()->SetFocus(math_t::Vec3f32(0, 0, 0)); }

	//draw the clouds?
		if (GetKeyboard()->IsKeyDown(input_hid::KeyboardEvent::c))
		{
			cloudFlag = 0;
			updateCloudFlag();
		}
		if (GetKeyboard()->IsKeyDown(input_hid::KeyboardEvent::x))
		{
			cloudFlag = 1;
			updateCloudFlag();
		}




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