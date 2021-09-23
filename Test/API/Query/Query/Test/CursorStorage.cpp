#include <Query/Test/CursorStorage.hpp>

namespace Emergence::Query::Test
{
std::string ObjectToString (const StandardLayout::Mapping &_mapping, const void *_object)
{
    const auto *current = static_cast<const uint8_t *> (_object);
    const auto *end = current + _mapping.GetObjectSize ();
    std::string result;

    while (current != end)
    {
        result += std::to_string (static_cast<std::size_t> (*current)) + " ";
        ++current;
    }

    return result;
}
} // namespace Emergence::Query::Test
