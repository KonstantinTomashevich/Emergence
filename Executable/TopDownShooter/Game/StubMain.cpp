#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Log/Log.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

BEGIN_MUTING_WARNINGS
#include <OgreApplicationContext.h>
#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
END_MUTING_WARNINGS

#include <Input/FixedInputMappingSingleton.hpp>
#include <Input/InputCollection.hpp>
#include <Input/NormalInputMappingSingleton.hpp>

#include <Shared/CelerityUtils.hpp>

#include <Time/TimeSingleton.hpp>
#include <Time/TimeSynchronization.hpp>

using namespace Emergence::Memory::Literals;

class WorldUpdater final
{
public:
    explicit WorldUpdater (OgreBites::ApplicationContext *_application, Emergence::Celerity::World *_world)
        : application (_application),
          modifyTime (_world->ModifySingletonExternally (TimeSingleton::Reflect ().mapping))
    {
        Emergence::Celerity::PipelineBuilder pipelineBuilder {_world};

        pipelineBuilder.Begin ("Fixed"_us);
        TimeSynchronization::AddFixedUpdateTasks (application->getRoot ()->getTimer (), pipelineBuilder);
        InputCollection::AddFixedUpdateTask (application, pipelineBuilder);
        Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
        fixedUpdate = pipelineBuilder.End (std::thread::hardware_concurrency ());

        pipelineBuilder.Begin ("Normal"_us);
        TimeSynchronization::AddNormalUpdateTasks (application->getRoot ()->getTimer (), pipelineBuilder);
        InputCollection::AddNormalUpdateTask (application, pipelineBuilder);
        Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
        normalUpdate = pipelineBuilder.End (std::thread::hardware_concurrency ());

        auto modifyFixedInput = _world->ModifySingletonExternally (FixedInputMappingSingleton::Reflect ().mapping);
        auto *fixedInput = static_cast<FixedInputMappingSingleton *> (*modifyFixedInput.Execute ());

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Forward"_us, "Movement"_us}, {{OgreBites::Keycode {'w'}, false}}});

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Left"_us, "Movement"_us}, {{OgreBites::Keycode {'a'}, false}}});

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Backward"_us, "Movement"_us}, {{OgreBites::Keycode {'s'}, false}}});

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Right"_us, "Movement"_us}, {{OgreBites::Keycode {'d'}, false}}});

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Explode"_us, "Ability"_us}, {{OgreBites::Keycode {'q'}, true}}});

        fixedInput->inputMapping.keyboardTriggers.EmplaceBack (KeyboardActionTrigger {
            InputAction {"Dash"_us, "Ability"_us}, {{OgreBites::Keycode {'w'}, false}, {OgreBites::SDLK_SPACE, true}}});

        auto modifyNormalInput = _world->ModifySingletonExternally (NormalInputMappingSingleton::Reflect ().mapping);
        auto *normalInput = static_cast<NormalInputMappingSingleton *> (*modifyNormalInput.Execute ());

        normalInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Up"_us, "Menu"_us}, {{OgreBites::Keycode {'1'}, true}}});

        normalInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Down"_us, "Menu"_us}, {{OgreBites::Keycode {'2'}, true}}});

        normalInput->inputMapping.keyboardTriggers.EmplaceBack (
            KeyboardActionTrigger {InputAction {"Confirm"_us, "Menu"_us}, {{OgreBites::Keycode {'e'}, true}}});

        auto *time = static_cast<TimeSingleton *> (*modifyTime.Execute ());
        time->targetFixedFrameDurationsS.EmplaceBack (1.0f / 60.0f);
        time->targetFixedFrameDurationsS.EmplaceBack (1.0f / 30.0f);
    }

    void Execute ()
    {
        // Intentionally lift write access to time singleton, because we are expecting changes from pipelines.
        auto *time = static_cast<TimeSingleton *> (*modifyTime.Execute ());
        application->pollEvents ();

        const uint64_t timeDifference = time->normalTimeUs > time->fixedTimeUs ?
                                            time->normalTimeUs - time->fixedTimeUs :
                                            time->fixedTimeUs - time->normalTimeUs;

        if (timeDifference < 1000000u)
        {
            while (time->fixedTimeUs < time->normalTimeUs)
            {
                fixedUpdate->Execute ();
            }
        }
        else
        {
            // Looks like we were sitting on breakpoint. In this case we will just assume that no time elapsed.
            time->fixedTimeUs = time->normalTimeUs;
        }

        normalUpdate->Execute ();
    }

private:
    OgreBites::ApplicationContext *application;
    Emergence::Warehouse::ModifySingletonQuery modifyTime;
    Emergence::Celerity::Pipeline *fixedUpdate = nullptr;
    Emergence::Celerity::Pipeline *normalUpdate = nullptr;
};

static void LogMemoryUsage (
    const Emergence::Memory::Profiler::AllocationGroup &_group = Emergence::Memory::Profiler::AllocationGroup::Root (),
    const Emergence::Container::String &_indentation = "")
{
    using namespace Emergence::Container;
    using namespace Emergence::Log;

    if (_group == Emergence::Memory::Profiler::AllocationGroup::Root ())
    {
        GlobalLogger::Log (
            Level::INFO,
            "Printing memory usage. Format: \"GroupName\": <AcquiredBytes> / <ReservedBytes> -- <UsagePercent>%");
    }

    const char *groupId = *_group.GetId () ? *_group.GetId () : "Root";
    const size_t percent = _group.GetTotal () ?
                               static_cast<size_t> (round (static_cast<float> (_group.GetAcquired ()) /
                                                           static_cast<float> (_group.GetTotal ()) * 100.0f)) :
                               0u;

    GlobalLogger::Log (Level::INFO, _indentation + "\"" + groupId + "\": " + ToString (_group.GetAcquired ()) + " / " +
                                        ToString (_group.GetTotal ()) + " -- " + ToString (percent) + "%");

    for (auto iterator = _group.BeginChildren (); iterator != _group.EndChildren (); ++iterator)
    {
        LogMemoryUsage (*iterator, _indentation + "    ");
    }
}

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

    bool usageLogged = false;

    {
        Emergence::Celerity::World world {"TestWorld"_us};
        WorldUpdater worldUpdater {&application, &world};

        while (!application.getRoot ()->endRenderingQueued ())
        {
            worldUpdater.Execute ();

            constexpr const uint64_t MS_PERIOD = 3000u;
            const uint64_t msGlobal = application.getRoot ()->getTimer ()->getMilliseconds ();
            const uint64_t msLocal = msGlobal % MS_PERIOD;
            const float angle = Ogre::Math::PI * 2.0f * (static_cast<float> (msLocal) / static_cast<float> (MS_PERIOD));

            Ogre::Vector3 lookTarget {cos (angle), -1.0f, sin (angle)};
            lightNode->lookAt (lookTarget, Ogre::Node::TS_WORLD);
            playerNode->setPosition (cos (angle) * 2.0f, 0.0f, sin (angle) * 2.0f);
            application.getRoot ()->renderOneFrame ();

            if (!usageLogged)
            {
                LogMemoryUsage ();
                usageLogged = true;
            }
        }
    }

    application.closeApp ();
    return 0u;
}
