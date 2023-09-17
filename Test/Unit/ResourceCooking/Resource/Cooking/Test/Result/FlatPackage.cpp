#include <Container/StringBuilder.hpp>

#include <Resource/Cooking/Pass/AllResourceImport.hpp>
#include <Resource/Cooking/Result/FlatPackage.hpp>
#include <Resource/Cooking/Test/Environment.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Cooking::Test;
using namespace Emergence::Resource::Cooking;

BEGIN_SUITE (ResultFlatPackage)

TEST_CASE (FlatPackage)
{
    Context context {GetResourceObjectTypes (), {}};
    PrepareEnvironmentAndSetupContext (
        context, {
                     {{"Building/B_Tower.yaml", {"Tower"_us, 3, 3}}, {"Building/B_Barrack.yaml", {"Barack"_us, 4, 2}}},
                     {{"Craft/C_HealingPotion.yaml", {"HealingPotion"_us, 5.0f, 1.0f}}},
                     {{"ThirdParty/Something.someformat", {13u, 11u, 111u, 174u, 186u, 9u, 7u, 3u}}},
                 });

    REQUIRE (AllResourceImportPass (context));
    REQUIRE (ProduceFlatPackage (context, "CoreResources.pack"));

    Emergence::VirtualFileSystem::Context checkSystem;
    REQUIRE (checkSystem.Mount (checkSystem.GetRoot (),
                                {Emergence::VirtualFileSystem::MountSource::PACKAGE,
                                 GetFinalResultRealPath (context, "CoreResources.pack"), "Package"}));

    // For simplicity, we just do existence check. Everything else is kind of guarantied by package builder.
    REQUIRE (Emergence::VirtualFileSystem::Entry {
        checkSystem.GetRoot (),
        EMERGENCE_BUILD_STRING ("Package", Emergence::VirtualFileSystem::PATH_SEPARATOR, "B_Barrack.yaml")});

    REQUIRE (Emergence::VirtualFileSystem::Entry {
        checkSystem.GetRoot (),
        EMERGENCE_BUILD_STRING ("Package", Emergence::VirtualFileSystem::PATH_SEPARATOR, "B_Tower.yaml")});

    REQUIRE (Emergence::VirtualFileSystem::Entry {
        checkSystem.GetRoot (),
        EMERGENCE_BUILD_STRING ("Package", Emergence::VirtualFileSystem::PATH_SEPARATOR, "C_HealingPotion.yaml")});

    REQUIRE (Emergence::VirtualFileSystem::Entry {
        checkSystem.GetRoot (),
        EMERGENCE_BUILD_STRING ("Package", Emergence::VirtualFileSystem::PATH_SEPARATOR, "Something.someformat")});
}

END_SUITE
