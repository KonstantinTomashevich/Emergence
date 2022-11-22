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
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (SecondComponent {0u, 100u, 12u, 1u}));
    parentBody.fullChangelist.emplace_back (MakePatch (FirstComponent {0u, 1.0f, 2.0f, 1.0f, 0.0f}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    childChangelist.emplace_back (MakePatch (FirstComponent {1u, 2.0f, 5.0f, 3.0f, 1.0f}));
    childChangelist.emplace_back (MakePatch (SecondComponent {1u, 112u, 10u, 4u}));

    Body childBody = ApplyInheritance (GetTypeManifest (), parentBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (parentBody.fullChangelist[0u]);
    expectedChildFullChangelist.emplace_back (parentBody.fullChangelist[1u]);
    expectedChildFullChangelist.emplace_back (childChangelist[0u]);
    expectedChildFullChangelist.emplace_back (childChangelist[1u]);

    CheckChangelistEquality (childBody.fullChangelist, expectedChildFullChangelist);
}

TEST_CASE (ApplyOverride)
{
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (SecondComponent {0u, 112u, 0u, 12u}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    childChangelist.emplace_back (MakePatch (SecondComponent {0u, 0u, 10u, 0u}));

    Body childBody = ApplyInheritance (GetTypeManifest (), parentBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (MakePatch (SecondComponent {0u, 112u, 10u, 12u}));

    CheckChangelistEquality (childBody.fullChangelist, expectedChildFullChangelist);
}

TEST_CASE (MultiApplyNoOverride)
{
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (MultiComponent {0u, 0u, "Warrior"_us, "WarriorRed"_us}));
    parentBody.fullChangelist.emplace_back (MakePatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorBlue"_us}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    childChangelist.emplace_back (MakePatch (MultiComponent {0u, 2u, "Mage"_us, "MageRed"_us}));
    childChangelist.emplace_back (MakePatch (MultiComponent {0u, 3u, "Mage"_us, "MageBlue"_us}));

    Body childBody = ApplyInheritance (GetTypeManifest (), parentBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (parentBody.fullChangelist[0u]);
    expectedChildFullChangelist.emplace_back (parentBody.fullChangelist[1u]);
    expectedChildFullChangelist.emplace_back (childChangelist[0u]);
    expectedChildFullChangelist.emplace_back (childChangelist[1u]);

    CheckChangelistEquality (childBody.fullChangelist, expectedChildFullChangelist);
}

TEST_CASE (MultiApplyOverride)
{
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorRed"_us}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    childChangelist.emplace_back (MakePatch (MultiComponent {0u, 1u, ""_us, "WarriorBlue"_us}));

    Body childBody = ApplyInheritance (GetTypeManifest (), parentBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildFullChangelist;
    expectedChildFullChangelist.emplace_back (MakePatch (MultiComponent {0u, 1u, "Warrior"_us, "WarriorBlue"_us}));

    CheckChangelistEquality (childBody.fullChangelist, expectedChildFullChangelist);
}

TEST_CASE (ExtractNoOverride)
{
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (SecondComponent {0u, 100u, 12u, 1u}));
    parentBody.fullChangelist.emplace_back (MakePatch (FirstComponent {0u, 1.0f, 2.0f, 1.0f, 0.0f}));

    Body childBody;
    childBody.fullChangelist.emplace_back (parentBody.fullChangelist[0u]);
    childBody.fullChangelist.emplace_back (parentBody.fullChangelist[1u]);
    childBody.fullChangelist.emplace_back (MakePatch (FirstComponent {1u, 5.0f, 4.0f, 3.0f, 1.0f}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    ExtractChildChangelist (GetTypeManifest (), parentBody, childBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildChangelist;
    expectedChildChangelist.emplace_back (childBody.fullChangelist[2u]);

    CheckChangelistEquality (childChangelist, expectedChildChangelist);
}

TEST_CASE (ExtractOverride)
{
    Body parentBody;
    parentBody.fullChangelist.emplace_back (MakePatch (SecondComponent {0u, 100u, 12u, 1u}));

    Body childBody;
    childBody.fullChangelist.emplace_back (MakePatch (SecondComponent {0u, 101u, 12u, 20u}));

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> childChangelist;
    ExtractChildChangelist (GetTypeManifest (), parentBody, childBody, childChangelist);

    Emergence::Container::Vector<Emergence::StandardLayout::Patch> expectedChildChangelist;
    expectedChildChangelist.emplace_back (MakePatch (SecondComponent {0u, 101u, 0u, 20u}));

    CheckChangelistEquality (childChangelist, expectedChildChangelist);
}

END_SUITE
