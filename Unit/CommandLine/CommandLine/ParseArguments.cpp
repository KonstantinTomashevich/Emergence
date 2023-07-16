#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <CommandLine/ParseArguments.hpp>

#include <Log/Log.hpp>

namespace Emergence::CommandLine
{
bool ParseArguments (int _argumentCount,
                     char **_arguments,
                     void *_output,
                     const StandardLayout::Mapping &_mapping) noexcept
{
    // We skip executable name.
    int index = 1;

    while (index < _argumentCount)
    {
        const std::string_view argument = _arguments[index];
        if (!argument.starts_with ("--") || argument.size () <= 2u)
        {
            EMERGENCE_LOG (ERROR,
                           "CommandLine::ParseArguments: Expected option in format \"--<optionName>\", but received \"",
                           argument, "\".");
            return false;
        }

        const std::string_view optionName = argument.substr (2u);
        StandardLayout::Field optionField;

        for (StandardLayout::Field field : _mapping)
        {
            if (optionName == *field.GetName ())
            {
                optionField = field;
                break;
            }
        }

        if (!optionField)
        {
            EMERGENCE_LOG (ERROR, "CommandLine::ParseArguments: Unable to find option \"", optionName, "\".");
            return false;
        }

        if (index + 1 >= _argumentCount)
        {
            EMERGENCE_LOG (ERROR, "CommandLine::ParseArguments: Unable to get value for option \"", optionName,
                           "\" as it is the last argument.");
            return false;
        }

        const std::string_view optionValue = _arguments[index + 1];
        switch (optionField.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
        {
            std::uint8_t value;
            if (optionValue == "1")
            {
                value = 1u << optionField.GetBitOffset ();
            }
            else if (optionValue == "0")
            {
                value = ~(1u << optionField.GetBitOffset ());
            }
            else
            {
                EMERGENCE_LOG (ERROR, "CommandLine::ParseArguments: Cannot convert \"", optionValue, "\" to bit.");
                return false;
            }

            if (value)
            {
                *static_cast<std::uint8_t *> (optionField.GetValue (_output)) |= value;
            }
            else
            {
                *static_cast<std::uint8_t *> (optionField.GetValue (_output)) &= value;
            }

            break;
        }

        case StandardLayout::FieldArchetype::INT:
        {
            long long value = std::stoll (optionValue.data ());
            switch (optionField.GetSize ())
            {
            case 1u:
                *static_cast<std::int8_t *> (optionField.GetValue (_output)) = static_cast<std::int8_t> (value);
                break;
            case 2u:
                *static_cast<std::int16_t *> (optionField.GetValue (_output)) = static_cast<std::int16_t> (value);
                break;
            case 4u:
                *static_cast<std::int32_t *> (optionField.GetValue (_output)) = static_cast<std::int32_t> (value);
                break;
            case 8u:
                *static_cast<std::int64_t *> (optionField.GetValue (_output)) = static_cast<std::int64_t> (value);
                break;
            }

            break;
        }

        case StandardLayout::FieldArchetype::UINT:
        {
            unsigned long long value = std::stoull (optionValue.data ());
            switch (optionField.GetSize ())
            {
            case 1u:
                *static_cast<std::uint8_t *> (optionField.GetValue (_output)) = static_cast<std::uint8_t> (value);
                break;
            case 2u:
                *static_cast<std::uint16_t *> (optionField.GetValue (_output)) = static_cast<std::uint16_t> (value);
                break;
            case 4u:
                *static_cast<std::uint32_t *> (optionField.GetValue (_output)) = static_cast<std::uint32_t> (value);
                break;
            case 8u:
                *static_cast<std::uint64_t *> (optionField.GetValue (_output)) = static_cast<std::uint64_t> (value);
                break;
            }

            break;
        }

        case StandardLayout::FieldArchetype::FLOAT:
        {
            double value = std::stod (optionValue.data ());
            switch (optionField.GetSize ())
            {
            case 4u:
                *static_cast<float *> (optionField.GetValue (_output)) = static_cast<float> (value);
                break;
            case 8u:
                *static_cast<double *> (optionField.GetValue (_output)) = static_cast<double> (value);
                break;
            }

            break;
        }

        case StandardLayout::FieldArchetype::STRING:
            strncpy (static_cast<char *> (optionField.GetValue (_output)), optionValue.data (),
                     optionField.GetSize () - 1u);
            static_cast<char *> (optionField.GetValue (_output))[optionField.GetSize () - 1u] = '\0';
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            *static_cast<Memory::UniqueString *> (optionField.GetValue (_output)) = Memory::UniqueString {optionValue};
            break;

        case StandardLayout::FieldArchetype::UTF8_STRING:
            *static_cast<Container::Utf8String *> (optionField.GetValue (_output)) = optionValue;
            break;

        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::VECTOR:
        case StandardLayout::FieldArchetype::PATCH:
            EMERGENCE_LOG (ERROR, "CommandLine::ParseArguments: Option \"", optionName,
                           "\" archetype is not supported.");
            return false;
        }

        index += 2;
    }

    return true;
}
} // namespace Emergence::CommandLine
