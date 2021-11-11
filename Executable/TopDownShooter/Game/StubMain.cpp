#include <Log/Log.hpp>

#include <OgreApplicationContext.h>
#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreTimer.h>

#include <Input/InputAccumulator.hpp>

int main (int /*unused*/, char ** /*unused*/)
{
    OgreBites::ApplicationContext application {"TopDownShooter"};
    application.initApp ();

    Ogre::Root *root = application.getRoot ();
    Ogre::SceneManager *sceneManager = root->createSceneManager ();

    Ogre::RTShader::ShaderGenerator *shaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr ();
    shaderGenerator->addSceneManager (sceneManager);

    Ogre::Light *light = sceneManager->createLight ("MainLight");
    light->setType (Ogre::Light::LT_DIRECTIONAL);
    light->setSpecularColour (Ogre::ColourValue::White);

    Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode ()->createChildSceneNode ();
    lightNode->lookAt ({0.0f, -2.0f, 0.0f}, Ogre::Node::TS_LOCAL);
    lightNode->attachObject (light);

    Ogre::Camera *camera = sceneManager->createCamera ("MainCamera");
    camera->setNearClipDistance (0.5f);
    camera->setAutoAspectRatio (true);

    Ogre::SceneNode *cameraNode = sceneManager->getRootSceneNode ()->createChildSceneNode ();
    cameraNode->setPosition (0, 5, 5);
    cameraNode->lookAt ({0.0f, 0.0f, -1.0f}, Ogre::Node::TS_WORLD);
    cameraNode->attachObject (camera);

    for (int x = -4; x <= 4; ++x)
    {
        for (int z = -4; z <= 4; ++z)
        {
            const bool placeWall = x == -4 || x == 4 || z == -4;
            Ogre::Entity *tile = sceneManager->createEntity (placeWall ? "Wall.mesh" : "FloorTile.mesh");

            Ogre::SceneNode *tileNode = sceneManager->getRootSceneNode ()->createChildSceneNode ();
            tileNode->setPosition (static_cast<float> (x), 0.0f, static_cast<float> (z));
            tileNode->attachObject (tile);
        }
    }

    Ogre::Entity *player = sceneManager->createEntity ("Player.mesh");
    Ogre::SceneNode *playerNode = sceneManager->getRootSceneNode ()->createChildSceneNode ();
    playerNode->setPosition (0.0f, 0.0f, 0.0f);
    playerNode->attachObject (player);

    application.getRenderWindow ()->addViewport (camera);

    {
        std::vector<KeyboardActionTrigger> keyboardActions {
            {InputAction {0u}, {{OgreBites::Keycode {'w'}, false}}},
            {InputAction {1u}, {{OgreBites::Keycode {'a'}, false}}},
            {InputAction {2u}, {{OgreBites::Keycode {'s'}, false}}},
            {InputAction {3u}, {{OgreBites::Keycode {'d'}, false}}},
            {InputAction {4u}, {{OgreBites::Keycode {'q'}, true}}},
            {InputAction {6u}, {{OgreBites::Keycode {'w'}, false}, {OgreBites::SDLK_SPACE, true}}}};

        InputAccumulator normalAccumulator {&application, keyboardActions};

        while (!application.getRoot ()->endRenderingQueued ())
        {
            constexpr const uint64_t MS_PERIOD = 3000u;
            const uint64_t msGlobal = application.getRoot ()->getTimer ()->getMilliseconds ();
            const uint64_t msLocal = msGlobal % MS_PERIOD;
            const float angle = Ogre::Math::PI * 2.0f * (static_cast<float> (msLocal) / static_cast<float> (MS_PERIOD));

            Ogre::Vector3 lookTarget {cos (angle), -1.0f, sin (angle)};
            lightNode->lookAt (lookTarget, Ogre::Node::TS_WORLD);

            playerNode->setPosition (cos (angle) * 2.0f, 0.0f, sin (angle) * 2.0f);

            InputAction action;
            while (normalAccumulator.PopNextAction (action, msGlobal))
            {
                using namespace Emergence::Log;
                GlobalLogger::Log (Level::INFO, "Received action: " + std::to_string (action.id) + ".");
            }

            application.getRoot ()->renderOneFrame ();
        }
    }

    application.closeApp ();
    return 0u;
}
