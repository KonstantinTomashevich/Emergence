#pragma warning(push, 0)
#include <OgreApplicationContext.h>
#include <OgreRoot.h>
#pragma warning(pop)

int main (int /*unused*/, char ** /*unused*/)
{
    OgreBites::ApplicationContext application {"TopDownShooter"};
    application.initApp ();
    application.getRoot ()->startRendering ();
    application.closeApp ();
    return 0u;
}
