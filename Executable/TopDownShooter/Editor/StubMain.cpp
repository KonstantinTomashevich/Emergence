#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Engine/Application.h>
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
    }

    void Start () override
    {
        Application::Start ();
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
