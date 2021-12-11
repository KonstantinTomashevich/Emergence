#include <SyntaxSugar/MuteWarnings.hpp>

#include <cassert>

BEGIN_MUTING_WARNINGS
#include <OgreRoot.h>
END_MUTING_WARNINGS

#include <Test/SharedApplicationContext.hpp>

class TestApplication final : public OgreBites::ApplicationContextBase
{
public:
    TestApplication (const Ogre::String &_name) : OgreBites::ApplicationContextBase (_name)
    {
    }

    // Base implementation of runtime config shows render selection dialog that blocks automatic testing.
    // Therefore, we override this method to select first available renderer and continue test execution.
    bool oneTimeConfig () final
    {
        assert (!mRoot->getAvailableRenderers ().empty ());
        mRoot->setRenderSystem (mRoot->getAvailableRenderers ().front ());
        return true;
    }
};

OgreBites::ApplicationContextBase *SharedApplicationContext::Get () noexcept
{
    static SharedApplicationContext sharedContext;
    return sharedContext.context;
}

SharedApplicationContext::SharedApplicationContext () noexcept
    : context (new TestApplication ("TopDownShooter::Shared::Test"))
{
    context->initApp ();
}

SharedApplicationContext::~SharedApplicationContext () noexcept
{
    context->closeApp ();
}
