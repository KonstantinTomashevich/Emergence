#include <MuteWarnings.hpp>

BEGIN_MUTING_WARNING
#include <OgreApplicationContext.h>
#include <OgreRoot.h>
END_MUTING_WARNING

int main (int /*unused*/, char ** /*unused*/)
{
    OgreBites::ApplicationContext application {"TopDownShooter"};
    application.initApp ();
    application.getRoot ()->startRendering ();
    application.closeApp ();
    return 0u;
}
