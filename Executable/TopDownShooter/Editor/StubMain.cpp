#include <OgreApplicationContext.h>
#include <OgreRoot.h>

int main (int /*unused*/, char ** /*unused*/)
{
    OgreBites::ApplicationContext application {"TopDownShooter"};
    application.initApp ();
    application.getRoot ()->startRendering ();
    application.closeApp ();
    return 0u;
}
