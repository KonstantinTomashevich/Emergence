#include <Log/Test/Shared.hpp>

namespace Emergence::Log::Test
{
static const std::string THREAD_PREFIX = "[Thread ";

std::string AddThreadIndexToMessage (std::size_t _index, const char *_message)
{
    return THREAD_PREFIX + std::to_string (_index) + "]: " + _message;
}

Container::Optional<std::size_t> ExtractThreadIndexFromMessage (const std::string &_message)
{
    std::size_t index = _message.find (THREAD_PREFIX);
    if (index == std::string::npos)
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
