#include <Container/StringBuilder.hpp>

#include <Resource/Object/Test/Helpers.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Resource::Object::Test
{
void CheckChangelistEquality (const Container::Vector<ObjectComponent> &_first,
                              const Container::Vector<ObjectComponent> &_second) noexcept
{
    // Comparing changelists correctly is difficult and not performance-friendly, as we need to take a lot
    // of details into account. Therefore, for test simplicity purposes we apply several simplifications:
    // - Patch order is guaranteed to be correct and matching.
    // - All object types have no paddings.
    // - All object types take 128 bytes of memory or less.

    std::array<uint8_t, 128u> firstBuffer;
    std::array<uint8_t, 128u> secondBuffer;
    CHECK_EQUAL (_first.size (), _second.size ());

    for (std::size_t index = 0u; index < std::min (_first.size (), _second.size ()); ++index)
    {
        const ObjectComponent &first = _first[index];
        const ObjectComponent &second = _second[index];

        if (first.component.GetTypeMapping () != second.component.GetTypeMapping ())
        {
            CHECK_WITH_MESSAGE (false, "Found mismatch at index ", index, ": first patch has type \"",
                                first.component.GetTypeMapping ().GetName (), "\", while second has type \"",
                                second.component.GetTypeMapping ().GetName (), "\".");
            continue;
        }

        REQUIRE (first.component.GetTypeMapping ().GetObjectSize () <= firstBuffer.size ());
        first.component.GetTypeMapping ().Construct (firstBuffer.data ());
        first.component.GetTypeMapping ().Construct (secondBuffer.data ());

        first.component.Apply (firstBuffer.data ());
        second.component.Apply (secondBuffer.data ());
        int comparisonResult =
            memcmp (firstBuffer.data (), secondBuffer.data (), first.component.GetTypeMapping ().GetObjectSize ());

        if (comparisonResult != 0)
        {
            CHECK_WITH_MESSAGE (false, "Found mismatch at index ", index,
                                ": Objects are not equal after patches are applied.\nFirst object: ",
                                EMERGENCE_BUILD_STRING (Container::StringBuilder::ObjectPointer {
                                    firstBuffer.data (), first.component.GetTypeMapping ()}),
                                "\nSecond object: ",
                                EMERGENCE_BUILD_STRING (Container::StringBuilder::ObjectPointer {
                                    secondBuffer.data (), first.component.GetTypeMapping ()}));
        }

        first.component.GetTypeMapping ().Destruct (firstBuffer.data ());
        first.component.GetTypeMapping ().Destruct (secondBuffer.data ());
    }
}
} // namespace Emergence::Resource::Object::Test
