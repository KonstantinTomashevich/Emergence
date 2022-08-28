#include <Asset/Object/Test/Helpers.hpp>

#include <Container/StringBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Asset::Object::Test
{
void CheckChangelistEquality (const Container::Vector<StandardLayout::Patch> &_first,
                              const Container::Vector<StandardLayout::Patch> &_second) noexcept
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
        const StandardLayout::Patch &first = _first[index];
        const StandardLayout::Patch &second = _second[index];

        if (first.GetTypeMapping () != second.GetTypeMapping ())
        {
            CHECK_WITH_MESSAGE (false, "Found mismatch at index ", index, ": first patch has type \"",
                                first.GetTypeMapping ().GetName (), "\", while second has type \"",
                                second.GetTypeMapping ().GetName (), "\".");
            continue;
        }

        REQUIRE (first.GetTypeMapping ().GetObjectSize () <= firstBuffer.size ());
        first.GetTypeMapping ().Construct (firstBuffer.data ());
        first.GetTypeMapping ().Construct (secondBuffer.data ());

        first.Apply (firstBuffer.data ());
        second.Apply (secondBuffer.data ());
        int comparisonResult =
            memcmp (firstBuffer.data (), secondBuffer.data (), first.GetTypeMapping ().GetObjectSize ());

        if (comparisonResult != 0)
        {
            CHECK_WITH_MESSAGE (false, "Found mismatch at index ", index,
                                ": Objects are not equal after patches are applied.\nFirst object: ",
                                EMERGENCE_BUILD_STRING (Container::StringBuilder::ObjectPointer {
                                    firstBuffer.data (), first.GetTypeMapping ()}),
                                "\nSecond object: ",
                                EMERGENCE_BUILD_STRING (Container::StringBuilder::ObjectPointer {
                                    secondBuffer.data (), first.GetTypeMapping ()}));
        }

        first.GetTypeMapping ().Destruct (firstBuffer.data ());
        first.GetTypeMapping ().Destruct (secondBuffer.data ());
    }
}
} // namespace Emergence::Asset::Object::Test
