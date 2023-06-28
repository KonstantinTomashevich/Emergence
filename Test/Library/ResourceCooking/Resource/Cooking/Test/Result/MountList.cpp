#include <Resource/Cooking/Result/MountList.hpp>
#include <Resource/Cooking/Test/Environment.hpp>

#include <Serialization/Binary.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Reader.hpp>

using namespace Emergence::Resource::Cooking::Test;
using namespace Emergence::Resource::Cooking;
using namespace Emergence::Serialization;

BEGIN_SUITE (ResultMountList)

TEST_CASE (MountList)
{
    Context context {GetResourceObjectTypes (), {}};
    PrepareEnvironmentAndSetupContext (context, {});

    const Emergence::VirtualFileSystem::MountConfigurationList testList {{
        {Emergence::VirtualFileSystem::MountSource::FILE_SYSTEM, "RawCoreResources", "CoreResources/Raw"},
        {Emergence::VirtualFileSystem::MountSource::PACKAGE, "CoreResources.pack", "CoreResources/Pack"},
    }};

    REQUIRE (ProduceMountList (context, "CoreResources", testList));
    Emergence::VirtualFileSystem::Reader reader {
        Emergence::VirtualFileSystem::Entry {context.GetFinalResultDirectory (), "MountCoreResources.bin"}};
    REQUIRE (reader);

    Emergence::VirtualFileSystem::MountConfigurationList resultList;
    REQUIRE (Binary::DeserializeObject (reader.InputStream (), &resultList,
                                        Emergence::VirtualFileSystem::MountConfigurationList::Reflect ().mapping, {}));
    CHECK (testList == resultList);
}

END_SUITE
