#include <Container/StringBuilder.hpp>

#include <Resource/Cooking/Pass/AllResourceImport.hpp>
#include <Resource/Cooking/Test/Environment.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Container;
using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Cooking::Test;
using namespace Emergence::Resource::Cooking;

BEGIN_SUITE (PassAllResourceImport)

TEST_CASE (AllResourceImport)
{
    Context context {GetResourceObjectTypes (), {}};
    PrepareEnvironmentAndSetupContext (
        context, {
                     {{"Building/B_Tower.yaml", {"Tower"_us, 3, 3}}, {"Building/B_Barrack.bin", {"Barack"_us, 4, 2}}},
                     {{"Craft/C_HealingPotion.yaml", {"HealingPotion"_us, 5.0f, 1.0f}}},
                     {{"ThirdParty/Something.someformat", {13u, 11u, 111u, 174u, 186u, 9u, 7u, 3u}}},
                 });

    REQUIRE (AllResourceImportPass (context));

    // For simplicity, we do not check the file contents and do not iterate the resource lists.
    Optional<ObjectData> objectData = context.GetSourceList ().QueryObject ("B_Tower"_us);
    REQUIRE (objectData);
    CHECK (objectData->type == FirstObjectType::Reflect ().mapping);
    CHECK (objectData->entry);
    CHECK (objectData->format == Emergence::Resource::Provider::ObjectFormat::YAML);

    objectData = context.GetSourceList ().QueryObject ("B_Barrack"_us);
    REQUIRE (objectData);
    CHECK (objectData->type == FirstObjectType::Reflect ().mapping);
    CHECK (objectData->entry);
    CHECK (objectData->format == Emergence::Resource::Provider::ObjectFormat::BINARY);

    objectData = context.GetSourceList ().QueryObject ("C_HealingPotion"_us);
    REQUIRE (objectData);
    CHECK (objectData->type == SecondObjectType::Reflect ().mapping);
    CHECK (objectData->entry);
    CHECK (objectData->format == Emergence::Resource::Provider::ObjectFormat::YAML);

    Optional<ThirdPartyData> thirdPartyData = context.GetSourceList ().QueryThirdParty ("Something.someformat"_us);
    REQUIRE (thirdPartyData);
    CHECK (thirdPartyData->entry);
}

END_SUITE
