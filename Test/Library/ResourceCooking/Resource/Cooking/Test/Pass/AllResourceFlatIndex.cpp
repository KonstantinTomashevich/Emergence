#include <Container/StringBuilder.hpp>

#include <Resource/Cooking/Pass/AllResourceFlatIndex.hpp>
#include <Resource/Cooking/Pass/AllResourceImport.hpp>
#include <Resource/Cooking/Result/FlatPackage.hpp>
#include <Resource/Cooking/Test/Environment.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Container;
using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Cooking::Test;
using namespace Emergence::Resource::Cooking;

BEGIN_SUITE (PassAllResourceFlatIndex)

TEST_CASE (AllResourceFlatIndex)
{
    Context context {GetResourceObjectTypes (), {}};
    PrepareEnvironmentAndSetupContext (
        context, {
                     {{"Building/B_Tower.yaml", {"Tower"_us, 3, 3}}, {"Building/B_Barrack.bin", {"Barack"_us, 4, 2}}},
                     {{"Craft/C_HealingPotion.yaml", {"HealingPotion"_us, 5.0f, 1.0f}}},
                     {{"ThirdParty/Something.someformat", {13u, 11u, 111u, 174u, 186u, 9u, 7u, 3u}}},
                 });

    REQUIRE (AllResourceImportPass (context));
    REQUIRE (AllResourceFlatIndexPass (context));

    // We need to produce something flat, so it is possible to check the index using resource provider loader.
    REQUIRE (ProduceFlatPackage (context, "CoreResources.pack"));

    Emergence::VirtualFileSystem::Context checkSystem;
    REQUIRE (checkSystem.Mount (checkSystem.GetRoot (),
                                {Emergence::VirtualFileSystem::MountSource::PACKAGE,
                                 GetFinalResultRealPath (context, "CoreResources.pack"), "Package"}));

    Emergence::Resource::Provider::ResourceProvider checkProvider {&checkSystem, GetResourceObjectTypes (), {}};
    REQUIRE (checkProvider.AddSource ("Package"_us) ==
             Emergence::Resource::Provider::SourceOperationResponse::SUCCESSFUL);

    // For simplicity, we just check loading without checking content.
    FirstObjectType firstObject;
    REQUIRE (checkProvider.LoadObject (FirstObjectType::Reflect ().mapping, "B_Tower"_us, &firstObject) ==
             Emergence::Resource::Provider::LoadingOperationResponse::SUCCESSFUL);

    REQUIRE (checkProvider.LoadObject (FirstObjectType::Reflect ().mapping, "B_Barrack"_us, &firstObject) ==
             Emergence::Resource::Provider::LoadingOperationResponse::SUCCESSFUL);

    SecondObjectType secondObject;
    REQUIRE (checkProvider.LoadObject (SecondObjectType::Reflect ().mapping, "C_HealingPotion"_us, &secondObject) ==
             Emergence::Resource::Provider::LoadingOperationResponse::SUCCESSFUL);

    Emergence::Memory::Heap heap {Emergence::Memory::Profiler::AllocationGroup::Top ()};
    std::uint8_t *content;
    std::uint64_t size;

    REQUIRE (checkProvider.LoadThirdPartyResource ("Something.someformat"_us, heap, size, content) ==
             Emergence::Resource::Provider::LoadingOperationResponse::SUCCESSFUL);
    heap.Release (content, static_cast<std::size_t> (size));
}

END_SUITE
