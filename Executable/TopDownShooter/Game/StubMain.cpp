#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Pipeline.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Log/Log.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/StreamSerializer.hpp>

BEGIN_MUTING_WARNINGS
#include <OgreApplicationContext.h>
#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreTimer.h>
END_MUTING_WARNINGS

#include <Input/Input.hpp>

#include <Shared/CelerityUtils.hpp>

using namespace Emergence::Memory::Literals;

static Emergence::Memory::Profiler::EventObserver StartRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

int main (int /*unused*/, char ** /*unused*/)
{
    // For testing purposes we start memory usage recording right away.
    std::ofstream memoryEventOutput {"MemoryRecording.track", std::ios::binary};
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;

    Emergence::Memory::Profiler::EventObserver memoryEventObserver =
        StartRecording (memoryEventSerializer, memoryEventOutput);

    OgreBites::ApplicationContext application {"TopDownShooter"};
    application.initApp ();

    Ogre::Root *root = application.getRoot ();
    Ogre::SceneManager *sceneManager = root->createSceneManager ();
    sceneManager->setAmbientLight (Ogre::ColourValue {0.2f, 0.2f, 0.2f});

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
        Emergence::Celerity::World world {"TestWorld"_us};
        Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};

        pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
        Input::AddToNormalUpdate (&application, pipelineBuilder);
        Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
        pipelineBuilder.End (std::thread::hardware_concurrency ());

        pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
        Input::AddToFixedUpdate (pipelineBuilder);
        Emergence::Celerity::AddAllCheckpoints (pipelineBuilder);
        pipelineBuilder.End (std::thread::hardware_concurrency ());

        while (!application.getRoot ()->endRenderingQueued ())
        {
            world.Update ();
            constexpr const uint64_t MS_PERIOD = 3000u;
            const uint64_t msGlobal = application.getRoot ()->getTimer ()->getMilliseconds ();
            const uint64_t msLocal = msGlobal % MS_PERIOD;
            const float angle = Ogre::Math::PI * 2.0f * (static_cast<float> (msLocal) / static_cast<float> (MS_PERIOD));

            Ogre::Vector3 lookTarget {cos (angle), -1.0f, sin (angle)};
            lightNode->lookAt (lookTarget, Ogre::Node::TS_WORLD);
            playerNode->setPosition (cos (angle) * 2.0f, 0.0f, sin (angle) * 2.0f);
            application.getRoot ()->renderOneFrame ();

            while (const Emergence::Memory::Profiler::Event *event = memoryEventObserver.NextEvent ())
            {
                memoryEventSerializer.SerializeEvent (*event);
            }
        }
    }

    application.closeApp ();
    return 0u;
}
