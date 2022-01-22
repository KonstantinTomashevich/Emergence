#include <Container/StringBuilder.hpp>

#include <Log/Test/Shared.hpp>

namespace Emergence::Log::Test
{
static const Container::String THREAD_PREFIX = "[Thread ";

Container::StringBuilder ConstructMessage (std::size_t _index, const char *_message)
{
    return EMERGENCE_BEGIN_BUILDING_STRING (THREAD_PREFIX, _index, "]: ", _message);
}

Container::Optional<std::size_t> ExtractThreadIndexFromMessage (const Container::String &_message)
{
    std::size_t index = _message.find (THREAD_PREFIX);
    if (index == Container::String::npos)
    {
        return std::nullopt;
    }

    static_assert (THREAD_COUNT <= 10u);
    const char indexChar = _message.at (index + THREAD_PREFIX.size ());

    if (indexChar >= '0' && indexChar <= '9')
    {
        return static_cast<std::size_t> (indexChar - '0');
    }

    return std::nullopt;
}
} // namespace Emergence::Log::Test
