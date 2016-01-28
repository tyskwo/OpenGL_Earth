#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>
#include <tlocPrefab/tloc_prefab.h>
#include <tlocApplication/tloc_application.h>

#include <gameAssetsPath.h>

using namespace tloc;

namespace {

#if defined (TLOC_OS_WIN)
    core_str::String shaderPathVS("/shaders/tlocOneTextureVS.glsl");
#elif defined (TLOC_OS_IPHONE)
    core_str::String shaderPathVS("/shaders/tlocOneTextureVS_gl_es_2_0.glsl");
#endif

#if defined (TLOC_OS_WIN)
    core_str::String shaderPathFS("/shaders/tlocOneTextureFS.glsl");
#elif defined (TLOC_OS_IPHONE)
    core_str::String shaderPathFS("/shaders/tlocOneTextureFS_gl_es_2_0.glsl");
#endif

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
    auto meshSys = scene->AddSystem<gfx_cs::MeshRenderSystem>();
    meshSys->SetRenderer(GetRenderer());

    //------------------------------------------------------------------------

    auto_cref to = app::resources::f_resource::LoadImageAsTextureObject
      (core_io::Path(g_assetsPath + "/images/engine_logo.png"));

    gfx_gl::uniform_vso  u_to;
    u_to->SetName("s_texture").SetValueAs(*to);

    //------------------------------------------------------------------------

    math_t::Rectf_c rect(math_t::Rectf_c::width(1.0f * 2.0f),
                         math_t::Rectf_c::height(GetWindow()->GetAspectRatio().Get() * 2.0f));

    core_cs::entity_vptr q = scene->CreatePrefab<pref_gfx::Quad>()
      .Dimensions(rect).Create();

    scene->CreatePrefab<pref_gfx::Material>()
      .AssetsPath(GetAssetsPath())
      .AddUniform(u_to.get())
      .Add(q, core_io::Path(shaderPathVS), core_io::Path(shaderPathFS));

    return Application::Post_Initialize();
  }
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

int TLOC_MAIN(int , char *[])
{
  Demo demo;
  demo.Initialize(core_ds::MakeTuple(800, 600));
  demo.Run();

  //------------------------------------------------------------------------
  // Exiting
  TLOC_LOG_CORE_INFO() << "Exiting normally";

  return 0;

}
