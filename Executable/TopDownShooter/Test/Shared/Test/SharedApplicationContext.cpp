#include <Test/SharedApplicationContext.hpp>

OgreBites::ApplicationContextBase *SharedApplicationContext::Get () noexcept
{
    static SharedApplicationContext sharedContext;
    return &sharedContext.context;
}

SharedApplicationContext::SharedApplicationContext () noexcept : context ("TopDownShooter::Shared::Test")
{
    // TODO: ogre.cfg must exists, otherwise app will ask for configuration file.
    context.initApp ();
}

SharedApplicationContext::~SharedApplicationContext () noexcept
{
    context.closeApp ();
}
