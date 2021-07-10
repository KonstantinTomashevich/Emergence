#include <ctime>
#include <variant>
#include <vector>

#include <Hashing/ByteHasher.hpp>
#include <Hashing/Test/ByteHasher.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Hashing::Test
{
bool ByteHasherTestIncludeMarker () noexcept
{
    return true;
}

struct AppendOne
{
    uint8_t byte;
};

struct AppendMany
{
    const uint8_t *bytes;
    std::size_t count;
};

struct Clear
{
};

using Scenario = std::vector <std::variant <AppendOne, AppendMany, Clear>>;

uint64_t ExecuteScenario (ByteHasher &_hasher, const Scenario &_scenario)
{
    for (const auto &operation : _scenario)
    {
        std::visit (
            [&_hasher] (const auto &unwrappedOperation)
            {
                using Type = std::decay_t <decltype (unwrappedOperation)>;
                if constexpr (std::is_same_v <Type, AppendOne>)
                {
                    _hasher.Append (static_cast <const AppendOne &> (unwrappedOperation).byte);
                }
                else if constexpr (std::is_same_v <Type, AppendMany>)
                {
                    const auto &operation = static_cast <const AppendMany &> (unwrappedOperation);
                    REQUIRE (operation.bytes);
                    _hasher.Append (operation.bytes, operation.count);
                }
                else if constexpr (std::is_same_v <Type, Clear>)
                {
                    _hasher.Clear ();
                }
            },
            operation);
    }

    return _hasher.GetCurrentValue ();
}

uint64_t ExecuteScenario (const Scenario &_scenario)
{
    ByteHasher hasher;
    return ExecuteScenario (hasher, _scenario);
}
} // namespace Emergence::Hashing::Test

BEGIN_SUITE (ByteHasher)

static const uint8_t firstSequence[] = {126u, 74u, 243u, 12u, 63u, 187u};

static const uint8_t secondSequence[] = {11u, 6u, 221u, 154u, 37u};

TEST_CASE (OneByte)
{
    Emergence::Hashing::Test::Scenario scenario =
        {
            Emergence::Hashing::Test::AppendOne {34u},
        };

    CHECK_EQUAL (Emergence::Hashing::Test::ExecuteScenario (scenario),
                       Emergence::Hashing::Test::ExecuteScenario (scenario));
}

TEST_CASE (OneByteSequences)
{
    CHECK_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {42u},
                Emergence::Hashing::Test::AppendOne {54u},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {42u},
                Emergence::Hashing::Test::AppendOne {54u},
            }));
}

TEST_CASE (OneSequence)
{
    Emergence::Hashing::Test::Scenario scenario =
        {
            Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
        };

    CHECK_EQUAL (Emergence::Hashing::Test::ExecuteScenario (scenario),
                       Emergence::Hashing::Test::ExecuteScenario (scenario));
}

TEST_CASE (OneCompositeSequence)
{
    Emergence::Hashing::Test::Scenario scenario =
        {
            Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
            Emergence::Hashing::Test::AppendMany {secondSequence, sizeof (secondSequence)},
        };

    CHECK_EQUAL (Emergence::Hashing::Test::ExecuteScenario (scenario),
                       Emergence::Hashing::Test::ExecuteScenario (scenario));
}

TEST_CASE (DifferentBytes)
{
    CHECK_NOT_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {34u},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {42u},
            }));
}

TEST_CASE (DifferentByteSequences)
{
    CHECK_NOT_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {34u},
                Emergence::Hashing::Test::AppendOne {54u},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {42u},
                Emergence::Hashing::Test::AppendOne {54u},
            }));
}

TEST_CASE (DifferentSequences)
{
    CHECK_NOT_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendMany {secondSequence, sizeof (secondSequence)},
            }));
}

TEST_CASE (DifferentCompositeSequences)
{
    CHECK_NOT_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
                Emergence::Hashing::Test::AppendMany {secondSequence, sizeof (secondSequence)},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendMany {secondSequence, sizeof (secondSequence)},
                Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
            }));
}

TEST_CASE (Clear)
{
    CHECK_EQUAL (
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {34u},
                Emergence::Hashing::Test::Clear {},
                Emergence::Hashing::Test::AppendOne {34u},
            }),
        Emergence::Hashing::Test::ExecuteScenario (
            {
                Emergence::Hashing::Test::AppendOne {42u},
                Emergence::Hashing::Test::Clear {},
                Emergence::Hashing::Test::AppendOne {34u},
            }));
}

TEST_CASE (DifferentSequencesThroughOneHasher)
{
    Emergence::Hashing::ByteHasher hasher;
    uint64_t firstResult = Emergence::Hashing::Test::ExecuteScenario (
        hasher,
        {
            Emergence::Hashing::Test::AppendMany {firstSequence, sizeof (firstSequence)},
        });

    hasher.Clear ();
    uint64_t secondResult = Emergence::Hashing::Test::ExecuteScenario (
        hasher,
        {
            Emergence::Hashing::Test::AppendMany {secondSequence, sizeof (secondSequence)},
        });

    CHECK_NOT_EQUAL (firstResult, secondResult);
}

TEST_CASE (VeryCloseBigRandomSequences)
{
    srand (time (nullptr));
    std::array <uint8_t, 512u> sequence;

    for (uint8_t &byte : sequence)
    {
        byte = static_cast <uint8_t> (rand ());
    }

    Emergence::Hashing::ByteHasher hasher;
    uint64_t firstResult = Emergence::Hashing::Test::ExecuteScenario (
        hasher,
        {
            Emergence::Hashing::Test::AppendMany {&sequence[0], sequence.size ()},
        });

    --sequence[sequence.size () / 2u];
    hasher.Clear ();

    uint64_t secondResult = Emergence::Hashing::Test::ExecuteScenario (
        hasher,
        {
            Emergence::Hashing::Test::AppendMany {&sequence[0], sequence.size ()},
        });

    CHECK_NOT_EQUAL (firstResult, secondResult);
}

END_SUITE