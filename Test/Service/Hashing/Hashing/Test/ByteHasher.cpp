#include <ctime>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Hashing/ByteHasher.hpp>
#include <Hashing/Test/ByteHasher.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Hashing::Test
{
bool ByteHasherTestIncludeMarker () noexcept
{
    return true;
}

struct AppendOne
{
    std::uint8_t byte;
};

struct AppendMany
{
    const std::uint8_t *bytes;
    std::size_t count;
};

struct Clear
{
};

using Scenario = Container::Vector<Container::Variant<AppendOne, AppendMany, Clear>>;

uint64_t ExecuteScenario (ByteHasher &_hasher, const Scenario &_scenario)
{
    for (const auto &operation : _scenario)
    {
        std::visit (
            [&_hasher] (const auto &_unwrappedOperation)
            {
                using Type = std::decay_t<decltype (_unwrappedOperation)>;
                if constexpr (std::is_same_v<Type, AppendOne>)
                {
                    _hasher.Append (static_cast<const AppendOne &> (_unwrappedOperation).byte);
                }
                else if constexpr (std::is_same_v<Type, AppendMany>)
                {
                    const auto &operation = static_cast<const AppendMany &> (_unwrappedOperation);
                    REQUIRE (operation.bytes);
                    _hasher.Append (operation.bytes, operation.count);
                }
                else if constexpr (std::is_same_v<Type, Clear>)
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

using namespace Emergence::Hashing::Test;

BEGIN_SUITE (ByteHasher)

static const std::uint8_t FIRST_SEQUENCE[] = {126u, 74u, 243u, 12u, 63u, 187u};

static const std::uint8_t SECOND_SEQUENCE[] = {11u, 6u, 221u, 154u, 37u};

TEST_CASE (OneByte)
{
    Scenario scenario = {AppendOne {34u}};
    CHECK_EQUAL (ExecuteScenario (scenario), ExecuteScenario (scenario));
}

TEST_CASE (OneByteSequences)
{
    CHECK_EQUAL (ExecuteScenario ({AppendOne {42u}, AppendOne {54u}}),
                 ExecuteScenario ({AppendOne {42u}, AppendOne {54u}}));
}

TEST_CASE (OneSequence)
{
    Scenario scenario = {AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)}};
    CHECK_EQUAL (ExecuteScenario (scenario), ExecuteScenario (scenario));
}

TEST_CASE (OneCompositeSequence)
{
    Scenario scenario = {
        AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)},
        AppendMany {SECOND_SEQUENCE, sizeof (SECOND_SEQUENCE)},
    };

    CHECK_EQUAL (ExecuteScenario (scenario), ExecuteScenario (scenario));
}

TEST_CASE (DifferentBytes)
{
    CHECK_NOT_EQUAL (ExecuteScenario ({AppendOne {34u}}), ExecuteScenario ({AppendOne {42u}}));
}

TEST_CASE (DifferentByteSequences)
{
    CHECK_NOT_EQUAL (ExecuteScenario ({AppendOne {34u}, AppendOne {54u}}),
                     ExecuteScenario ({AppendOne {42u}, AppendOne {54u}}));
}

TEST_CASE (DifferentSequences)
{
    CHECK_NOT_EQUAL (ExecuteScenario ({AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)}}),
                     ExecuteScenario ({AppendMany {SECOND_SEQUENCE, sizeof (SECOND_SEQUENCE)}}));
}

TEST_CASE (DifferentCompositeSequences)
{
    CHECK_NOT_EQUAL (ExecuteScenario ({AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)},
                                       AppendMany {SECOND_SEQUENCE, sizeof (SECOND_SEQUENCE)}}),
                     ExecuteScenario ({AppendMany {SECOND_SEQUENCE, sizeof (SECOND_SEQUENCE)},
                                       AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)}}));
}

TEST_CASE (Clear)
{
    CHECK_EQUAL (ExecuteScenario ({AppendOne {34u}, Clear {}, AppendOne {34u}}),
                 ExecuteScenario ({AppendOne {42u}, Clear {}, AppendOne {34u}}));
}

TEST_CASE (DifferentSequencesThroughOneHasher)
{
    Emergence::Hashing::ByteHasher hasher;
    std::uint64_t firstResult = ExecuteScenario (hasher, {AppendMany {FIRST_SEQUENCE, sizeof (FIRST_SEQUENCE)}});

    hasher.Clear ();
    std::uint64_t secondResult = ExecuteScenario (hasher, {AppendMany {SECOND_SEQUENCE, sizeof (SECOND_SEQUENCE)}});

    CHECK_NOT_EQUAL (firstResult, secondResult);
}

TEST_CASE (VeryCloseBigRandomSequences)
{
    srand (static_cast<unsigned int> (time (nullptr)));
    std::array<std::uint8_t, 512u> sequence;

    for (std::uint8_t &byte : sequence)
    {
        byte = static_cast<std::uint8_t> (rand ());
    }

    Emergence::Hashing::ByteHasher hasher;
    std::uint64_t firstResult = ExecuteScenario (hasher, {AppendMany {sequence.data (), sequence.size ()}});

    --sequence[sequence.size () / 2u];
    hasher.Clear ();

    std::uint64_t secondResult = ExecuteScenario (hasher, {AppendMany {sequence.data (), sequence.size ()}});
    CHECK_NOT_EQUAL (firstResult, secondResult);
}

TEST_CASE (CopyConstruct)
{
    Emergence::Hashing::ByteHasher source;
    source.Append (13u);

    Emergence::Hashing::ByteHasher copy (source);
    source.Append (72u);
    copy.Append (72u);
    CHECK_EQUAL (source.GetCurrentValue (), copy.GetCurrentValue ());
}

TEST_CASE (MoveConstruct)
{
    Emergence::Hashing::ByteHasher source;
    source.Append (13u);

    Emergence::Hashing::ByteHasher target (std::move (source));
    target.Append (72u);
    CHECK_EQUAL (target.GetCurrentValue (), ExecuteScenario ({AppendOne {13u}, AppendOne {72u}}));
}

TEST_CASE (CopyAssign)
{
    Emergence::Hashing::ByteHasher source;
    Emergence::Hashing::ByteHasher target;

    source.Append (13u);
    target.Append (12u);

    target = source;
    source.Append (72u);
    target.Append (72u);
    CHECK_EQUAL (source.GetCurrentValue (), target.GetCurrentValue ());
}

TEST_CASE (MoveAssign)
{
    Emergence::Hashing::ByteHasher source;
    Emergence::Hashing::ByteHasher target;

    source.Append (13u);
    target.Append (12u);

    target = std::move (source);
    target.Append (72u);
    CHECK_EQUAL (target.GetCurrentValue (), ExecuteScenario ({AppendOne {13u}, AppendOne {72u}}));
}

END_SUITE
