#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Object/Test/Helpers.hpp>
#include <Resource/Object/Test/Types.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::Resource::Object::Test;
using namespace Emergence::Resource::Object;

BEGIN_SUITE (ObjectInheritance)

TEST_CASE (ApplyNoOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 100u, 12u, 1u}));
    parentObject.changelist.emplace_back (MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 1.0f, 0.0f}));

    Object childObject;
    childObject.changelist.emplace_back (MakeComponentPatch (FirstComponent {1u, 2.0f, 5.0f, 3.0f, 1.0f}));
    childObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {1u, 112u, 10u, 4u}));

    Emergence::Container::Vector<ObjectComponent> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (parentObject.changelist[0u]);
    expectedChildFullChangelist.emplace_back (parentObject.changelist[1u]);
    expectedChildFullChangelist.emplace_back (childObject.changelist[0u]);
    expectedChildFullChangelist.emplace_back (childObject.changelist[1u]);

    ApplyInheritance (GetTypeManifest (), parentObject, childObject);
    CheckChangelistEquality (childObject.changelist, expectedChildFullChangelist);
}

TEST_CASE (ApplyOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 112u, 0u, 12u}));

    Object childObject;
    childObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 0u, 10u, 0u}));

    Emergence::Container::Vector<ObjectComponent> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 112u, 10u, 12u}));

    ApplyInheritance (GetTypeManifest (), parentObject, childObject);
    CheckChangelistEquality (childObject.changelist, expectedChildFullChangelist);
}

TEST_CASE (MultiApplyNoOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 0u, "Warrior"_us, "WarriorRed"_us}));
    parentObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorBlue"_us}));

    Object childObject;
    childObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 2u, "Mage"_us, "MageRed"_us}));
    childObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 3u, "Mage"_us, "MageBlue"_us}));

    Emergence::Container::Vector<ObjectComponent> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (parentObject.changelist[0u]);
    expectedChildFullChangelist.emplace_back (parentObject.changelist[1u]);
    expectedChildFullChangelist.emplace_back (childObject.changelist[0u]);
    expectedChildFullChangelist.emplace_back (childObject.changelist[1u]);

    ApplyInheritance (GetTypeManifest (), parentObject, childObject);
    CheckChangelistEquality (childObject.changelist, expectedChildFullChangelist);
}

TEST_CASE (MultiApplyOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorRed"_us}));

    Object childObject;
    childObject.changelist.emplace_back (MakeComponentPatch (MultiComponent {0u, 1u, ""_us, "WarriorBlue"_us}));

    Emergence::Container::Vector<ObjectComponent> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (
        MakeComponentPatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorBlue"_us}));

    ApplyInheritance (GetTypeManifest (), parentObject, childObject);
    CheckChangelistEquality (childObject.changelist, expectedChildFullChangelist);
}

TEST_CASE (ExtractNoOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 100u, 12u, 1u}));
    parentObject.changelist.emplace_back (MakeComponentPatch (FirstComponent {0u, 1.0f, 2.0f, 1.0f, 0.0f}));

    Object childObject;
    childObject.changelist.emplace_back (parentObject.changelist[0u]);
    childObject.changelist.emplace_back (parentObject.changelist[1u]);
    childObject.changelist.emplace_back (MakeComponentPatch (FirstComponent {1u, 5.0f, 4.0f, 3.0f, 1.0f}));

    Emergence::Container::Vector<ObjectComponent> extractedChildChangelist;
    ExtractChildChangelist (GetTypeManifest (), parentObject, childObject, extractedChildChangelist);

    Emergence::Container::Vector<ObjectComponent> expectedChildChangelist;
    expectedChildChangelist.emplace_back (childObject.changelist[2u]);

    CheckChangelistEquality (extractedChildChangelist, expectedChildChangelist);
}

TEST_CASE (ExtractOverride)
{
    Object parentObject;
    parentObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 100u, 12u, 1u}));

    Object childObject;
    childObject.changelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 101u, 12u, 20u}));

    Emergence::Container::Vector<ObjectComponent> extractedChildChangelist;
    ExtractChildChangelist (GetTypeManifest (), parentObject, childObject, extractedChildChangelist);

    Emergence::Container::Vector<ObjectComponent> expectedChildChangelist;
    expectedChildChangelist.emplace_back (MakeComponentPatch (SecondComponent {0u, 101u, 0u, 20u}));

    CheckChangelistEquality (extractedChildChangelist, expectedChildChangelist);
}

END_SUITE
