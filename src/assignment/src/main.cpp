#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>

#include <gameAssetsPath.h>

using namespace tloc;

namespace {
	core_str::String shaderPathVS("/shaders/multipleTexturesVS.glsl");

	core_str::String shaderPathFS("/shaders/multipleTexturesFS.glsl");


	const core_str::String g_assetsPath(GetAssetsPath());

};

// ///////////////////////////////////////////////////////////////////////
// Demo app

class Demo
	: public Application
{
public:
	Demo()
		: Application("2LoC Engine")
	{ }

private:
	error_type Post_Initialize() override
	{
		auto& scene = GetScene();
		scene->AddSystem<gfx_cs::MaterialSystem>();
		scene->AddSystem<gfx_cs::CameraSystem>();

		auto meshSys = scene->AddSystem<gfx_cs::MeshRenderSystem>();
		meshSys->SetRenderer(GetRenderer());

		core_str::String assetPath = GetAssetsPath();

		auto quadMesh = scene->CreatePrefab<pref_gfx::Quad>()
			.Create();
		quadMesh->SetDebugName("QuadMesh");

		scene->CreatePrefab<pref_gfx::Material>()
			.Add(quadMesh, core_io::Path(assetPath + shaderPathVS),
			core_io::Path(assetPath + shaderPathFS));

		auto matComp = quadMesh->GetComponent<gfx_cs::Material>();
		auto so = matComp->GetShaderOperator();

		auto texObj_1 = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(assetPath + "/images/engine_logo.png"));
		auto texObj_2 = app_res::f_resource::LoadImageAsTextureObject(core_io::Path(assetPath + "/images/brick_diff.jpg"));

		gfx_gl::uniform_vso u_firstTexture;
		u_firstTexture->SetName("s_texture").SetValueAs(*texObj_1);

		gfx_gl::uniform_vso u_secondTexture;
		u_secondTexture->SetName("s_texture_2").SetValueAs(*texObj_2);

		so->AddUniform(*u_firstTexture);
		so->AddUniform(*u_secondTexture);


		auto camEnt = scene->CreatePrefab<pref_gfx::Camera>()
			.Perspective(true)
			.Near(0.1f)
			.Far(100.0f)
			.VerticalFOV(math_t::Degree(60.0f))
			.Create(GetWindow()->GetDimensions());


		camEnt->GetComponent<math_cs::Transform>()->SetPosition(math_t::Vec3f32(0, 0, 2));

		meshSys->SetCamera(camEnt);


		return Application::Post_Initialize();
	}
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

int TLOC_MAIN(int, char *[])
{
	Demo demo;
	demo.Initialize(core_ds::MakeTuple(800, 600));
	demo.Run();

	//------------------------------------------------------------------------
	// Exiting
	TLOC_LOG_CORE_INFO() << "Exiting normally";

	return 0;

}
