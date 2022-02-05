#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Container/Str.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
END_MUTING_WARNINGS

class EditorApplication : public Urho3D::Application
{
    URHO3D_OBJECT (EditorApplication, Application)

public:
    EditorApplication (Urho3D::Context *_context) : Application (_context)
    {
    }

    EditorApplication (const EditorApplication &_other) = delete;

    EditorApplication (EditorApplication &&_other) = delete;

    ~EditorApplication () override = default;

    void Setup () override
    {
        Application::Setup ();
        engineParameters_[Urho3D::EP_FULL_SCREEN] = false;
        engineParameters_[Urho3D::EP_RESOURCE_PATHS] = "Urho3DCoreAssets;GameAssets";
        engineParameters_[Urho3D::EP_RESOURCE_PREFIX_PATHS] = "..";
    }

    void Start () override
    {
        Application::Start ();
        auto *input = GetSubsystem<Urho3D::Input> ();
        input->SetMouseVisible (true);
        input->SetMouseMode (Urho3D::MM_FREE);
    }

    void Stop () override
    {
        Application::Stop ();
    }

    EditorApplication &operator= (const EditorApplication &_other) = delete;

    EditorApplication &operator= (EditorApplication &&_other) = delete;
};

BEGIN_MUTING_WARNINGS
URHO3D_DEFINE_APPLICATION_MAIN (EditorApplication)
END_MUTING_WARNINGS
