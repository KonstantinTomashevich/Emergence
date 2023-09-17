#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <Serialization/Yaml.hpp>

#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

// We're linking to static library, but define is not passed to us for some reason.
// Therefore, we need to add it manually.
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace Emergence::Serialization::Yaml
{
void SerializeTypeName (std::ostream &_output, Memory::UniqueString _typeName) noexcept
{
    _output << "# " << *_typeName << std::endl;
}

Memory::UniqueString DeserializeTypeName (std::istream &_input) noexcept
{
    Container::StringBuilder typeName;
    while (_input)
    {
        int next = _input.get ();
        if (next == '\n' || next == '\r')
        {
            break;
        }

        typeName.Append (static_cast<char> (next));
    }

    if (!_input || typeName.GetCount () < 3u || typeName.Get ()[0u] != '#' || typeName.Get ()[1u] != ' ' ||
        typeName.Get ()[2u] == '\n' || typeName.Get ()[2u] == '\r')
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Failed to parse type name. Parsed sequence: \"", typeName.Get (),
                       "\".");
        return {};
    }

    return Memory::UniqueString {typeName.Get () + 2u};
}

static YAML::Node SerializeLeafValueToYaml (const void *_address, const StandardLayout::Field &_field)
{
    YAML::Node result {YAML::NodeType::Scalar};
    switch (_field.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:

        // We are casting to bool to signal that this value must be serialized as boolean.
        result = static_cast<bool> (*static_cast<const std::uint8_t *> (_address) & (1u << _field.GetBitOffset ()));
        break;

    case StandardLayout::FieldArchetype::INT:
        switch (_field.GetSize ())
        {
        case 1u:
            // We are converting to 16-bit int to avoid char conversion.
            result = static_cast<int16_t> (*static_cast<const int8_t *> (_address));
            break;

        case 2u:
            result = *static_cast<const int16_t *> (_address);
            break;

        case 4u:
            result = *static_cast<const int32_t *> (_address);
            break;

        case 8u:
            result = *static_cast<const int64_t *> (_address);
            break;
        }
        break;

    case StandardLayout::FieldArchetype::UINT:
        switch (_field.GetSize ())
        {
        case 1u:
            // We are converting to 16-bit int to avoid char conversion.
            result = static_cast<std::uint16_t> (*static_cast<const std::uint8_t *> (_address));
            break;

        case 2u:
            result = *static_cast<const std::uint16_t *> (_address);
            break;

        case 4u:
            result = *static_cast<const std::uint32_t *> (_address);
            break;

        case 8u:
            result = *static_cast<const std::uint64_t *> (_address);
            break;
        }
        break;

    case StandardLayout::FieldArchetype::FLOAT:
        switch (_field.GetSize ())
        {
        case 4u:
            result = *static_cast<const float *> (_address);
            break;

        case 8u:
            result = *static_cast<const double *> (_address);
            break;
        }
        break;

    case StandardLayout::FieldArchetype::STRING:
        result = static_cast<const char *> (_address);
        break;

    case StandardLayout::FieldArchetype::BLOCK:
        result = YAML::Binary (static_cast<const unsigned char *> (_address), _field.GetSize ());
        break;

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
    {
        const char *stringValue = **static_cast<const Memory::UniqueString *> (_address);
        if (stringValue)
        {
            result = stringValue;
        }
        else
        {
            result = "";
        }

        break;
    }

    case StandardLayout::FieldArchetype::UTF8_STRING:
    {
        const char *stringValue = static_cast<const Container::Utf8String *> (_address)->c_str ();
        if (stringValue)
        {
            result = stringValue;
        }
        else
        {
            result = "";
        }

        break;
    }

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
    case StandardLayout::FieldArchetype::VECTOR:
    case StandardLayout::FieldArchetype::PATCH:
        // Only leaf values are supported.
        EMERGENCE_ASSERT (false);
        break;
    }

    return result;
}

void SerializePatchValue (const StandardLayout::Field &_field,
                          const void *_newValue,
                          YAML::Node &_node,
                          const char *_fieldNameIterator)
{
    const char *position = strchr (_fieldNameIterator, StandardLayout::PROJECTION_NAME_SEPARATOR);
    if (position)
    {
        std::string_view nodeName {_fieldNameIterator, position};
        // We need to allocate string for node name, because yaml-cpp does not support string views.
        YAML::Node child = _node[std::string {nodeName}];
        SerializePatchValue (_field, _newValue, child, position + 1u);
    }
    else
    {
        _node[_fieldNameIterator] = SerializeLeafValueToYaml (_newValue, _field);
    }
}

static void SerializeObjectToYaml (YAML::Node &_output, const void *_object, const StandardLayout::Mapping &_mapping)
{
    for (auto iterator = _mapping.BeginConditional (_object), end = _mapping.EndConditional (); iterator != end;
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        if (field.IsProjected ())
        {
            continue;
        }

        if (field.GetArchetype () == StandardLayout::FieldArchetype::NESTED_OBJECT)
        {
            YAML::Node node {YAML::NodeType::Map};
            SerializeObjectToYaml (node, field.GetValue (_object), field.GetNestedObjectMapping ());
            _output[*field.GetName ()] = node;
        }
        else if (field.GetArchetype () == StandardLayout::FieldArchetype::VECTOR)
        {
            YAML::Node node {YAML::NodeType::Sequence};
            const void *vectorAddress = field.GetValue (_object);

            for (const std::uint8_t *pointer = Container::UntypedVectorUtility::Begin (vectorAddress);
                 pointer != Container::UntypedVectorUtility::End (vectorAddress);
                 pointer += field.GetVectorItemMapping ().GetObjectSize ())
            {
                YAML::Node itemNode {YAML::NodeType::Map};
                SerializeObjectToYaml (itemNode, pointer, field.GetVectorItemMapping ());
                node.push_back (itemNode);
            }

            _output[*field.GetName ()] = node;
        }
        else if (field.GetArchetype () == StandardLayout::FieldArchetype::PATCH)
        {
            const void *patchAddress = field.GetValue (_object);
            const StandardLayout::Patch &patch = *static_cast<const StandardLayout::Patch *> (patchAddress);

            YAML::Node node {YAML::NodeType::Map};
            node["type"] = *patch.GetTypeMapping ().GetName ();
            YAML::Node content {YAML::NodeType::Map};

            for (const StandardLayout::Patch::ChangeInfo &change : patch)
            {
                StandardLayout::Field patchField = patch.GetTypeMapping ().GetField (change.field);
                SerializePatchValue (patchField, change.newValue, content, *patchField.GetName ());
            }

            node["content"] = content;
            _output[*field.GetName ()] = node;
        }
        else
        {
            _output[*field.GetName ()] = SerializeLeafValueToYaml (field.GetValue (_object), field);
        }
    }
}

static bool DeserializeLeafValueFromYaml (const YAML::Node &_input, void *_address, const StandardLayout::Field &_field)
{
    try
    {
        switch (_field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
            if (_input.as<bool> ())
            {
                *static_cast<std::uint8_t *> (_address) |= 1u << _field.GetBitOffset ();
            }
            else
            {
                *static_cast<std::uint8_t *> (_address) &= ~(1u << _field.GetBitOffset ());
            }
            break;

        case StandardLayout::FieldArchetype::INT:
            switch (_field.GetSize ())
            {
            case 1u:
                *static_cast<int8_t *> (_address) = _input.as<int8_t> ();
                break;
            case 2u:
                *static_cast<int16_t *> (_address) = _input.as<int16_t> ();
                break;
            case 4u:
                *static_cast<int32_t *> (_address) = _input.as<int32_t> ();
                break;
            case 8u:
                *static_cast<int64_t *> (_address) = _input.as<int64_t> ();
                break;
            }
            break;

        case StandardLayout::FieldArchetype::UINT:
            switch (_field.GetSize ())
            {
            case 1u:
                *static_cast<std::uint8_t *> (_address) = _input.as<std::uint8_t> ();
                break;
            case 2u:
                *static_cast<std::uint16_t *> (_address) = _input.as<std::uint16_t> ();
                break;
            case 4u:
                *static_cast<std::uint32_t *> (_address) = _input.as<std::uint32_t> ();
                break;
            case 8u:
                *static_cast<std::uint64_t *> (_address) = _input.as<std::uint64_t> ();
                break;
            }
            break;

        case StandardLayout::FieldArchetype::FLOAT:
            switch (_field.GetSize ())
            {
            case 4u:
                *static_cast<float *> (_address) = _input.as<float> ();
                break;
            case 8u:
                *static_cast<double *> (_address) = _input.as<double> ();
                break;
            }
            break;

        case StandardLayout::FieldArchetype::STRING:
            strncpy (static_cast<char *> (_address), _input.Scalar ().c_str (), _field.GetSize () - 1u);
            static_cast<char *> (_address)[_field.GetSize () - 1u] = '\0';
            break;

        case StandardLayout::FieldArchetype::BLOCK:
        {
            auto binary = _input.as<YAML::Binary> ();
            EMERGENCE_ASSERT (binary.data ());
            memcpy (_address, binary.data (), std::min (binary.size (), _field.GetSize ()));
            break;
        }

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            *static_cast<Memory::UniqueString *> (_address) = Memory::UniqueString {_input.Scalar ().c_str ()};
            break;

        case StandardLayout::FieldArchetype::UTF8_STRING:
            *static_cast<Container::Utf8String *> (_address) = _input.Scalar ();
            break;

        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::VECTOR:
        case StandardLayout::FieldArchetype::PATCH:
            // Only leaf values are supported.
            EMERGENCE_ASSERT (false);
            break;
        }
    }
    catch ([[maybe_unused]] YAML::Exception &exception)
    {
        return false;
    }

    return true;
}

static bool DeserializePatchLeafValueFromYaml (const YAML::Node &_input,
                                               StandardLayout::PatchBuilder &_builder,
                                               const StandardLayout::Mapping &_mapping,
                                               const StandardLayout::Field &_field)
{
    StandardLayout::FieldId fieldId = _mapping.GetFieldId (_field);
    try
    {
        switch (_field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
            _builder.SetBit (fieldId, _input.as<bool> ());
            break;

        case StandardLayout::FieldArchetype::INT:
            switch (_field.GetSize ())
            {
            case 1u:
                _builder.SetInt8 (fieldId, _input.as<int8_t> ());
                break;
            case 2u:
                _builder.SetInt16 (fieldId, _input.as<int16_t> ());
                break;
            case 4u:
                _builder.SetInt32 (fieldId, _input.as<int32_t> ());
                break;
            case 8u:
                _builder.SetInt64 (fieldId, _input.as<int64_t> ());
                break;
            }
            break;

        case StandardLayout::FieldArchetype::UINT:
            switch (_field.GetSize ())
            {
            case 1u:
                _builder.SetUInt8 (fieldId, _input.as<std::uint8_t> ());
                break;
            case 2u:
                _builder.SetUInt16 (fieldId, _input.as<std::uint16_t> ());
                break;
            case 4u:
                _builder.SetUInt32 (fieldId, _input.as<std::uint32_t> ());
                break;
            case 8u:
                _builder.SetUInt64 (fieldId, _input.as<std::uint64_t> ());
                break;
            }
            break;

        case StandardLayout::FieldArchetype::FLOAT:
            switch (_field.GetSize ())
            {
            case 4u:
                _builder.SetFloat (fieldId, _input.as<float> ());
                break;
            case 8u:
                _builder.SetDouble (fieldId, _input.as<double> ());
                break;
            }
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            _builder.SetUniqueString (fieldId, Memory::UniqueString {_input.Scalar ().c_str ()});
            break;

        case StandardLayout::FieldArchetype::STRING:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::UTF8_STRING:
        case StandardLayout::FieldArchetype::VECTOR:
        case StandardLayout::FieldArchetype::PATCH:
            // Unsupported for patches.
            EMERGENCE_ASSERT (false);
            break;
        }
    }
    catch ([[maybe_unused]] YAML::Exception &exception)
    {
        return false;
    }

    return true;
}

static StandardLayout::Field FindFieldByName (const StandardLayout::Mapping &_mapping, Memory::UniqueString _fieldName)
{
    for (StandardLayout::Field field : _mapping)
    {
        if (field.GetName () == _fieldName)
        {
            return field;
        }
    }

    return {};
}

static bool DeserializePatchBodyFromYaml (const YAML::Node &_input,
                                          const StandardLayout::Mapping &_mapping,
                                          StandardLayout::PatchBuilder &_builder,
                                          const Container::String &_prefix)
{
    for (auto iterator = _input.begin (); iterator != _input.end (); ++iterator)
    {
        if (!iterator->first.IsScalar ())
        {
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered map pair with non-scalar key!");
            return false;
        }

        switch (iterator->second.Type ())
        {
        case YAML::NodeType::Undefined:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered undefined node during iteration!");
            return false;

        case YAML::NodeType::Null:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered null node during iteration!");
            return false;

        case YAML::NodeType::Scalar:
        {
            // NOLINTNEXTLINE(readability-redundant-string-cstr): It's not, because allocators are different.
            const Memory::UniqueString fieldName {_prefix + iterator->first.Scalar ().c_str ()};
            StandardLayout::Field field = FindFieldByName (_mapping, fieldName);

            if (!field)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Mapping \"", _mapping.GetName (),
                               "\" does not contain field \"", *fieldName, "\"!");
                return false;
            }

            if (!DeserializePatchLeafValueFromYaml (iterator->second, _builder, _mapping, field))
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to deserialize value of field \"", *fieldName,
                               "\" from mapping \"", _mapping.GetName (), "\"!");
                return false;
            }

            break;
        }

        case YAML::NodeType::Sequence:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered sequence node during iteration!");
            return false;

        case YAML::NodeType::Map:
        {
            if (!DeserializePatchBodyFromYaml (
                    iterator->second, _mapping, _builder,
                    // NOLINTNEXTLINE(readability-redundant-string-cstr): It's not, because allocators are different.
                    _prefix + iterator->first.Scalar ().c_str () + StandardLayout::PROJECTION_NAME_SEPARATOR))
            {
                return false;
            }

            break;
        }
        }
    }

    return true;
}

static bool DeserializeObjectFromYaml (const YAML::Node &_input,
                                       void *_objectAddress,
                                       const StandardLayout::Mapping &_mapping,
                                       const Container::MappingRegistry &_patchableTypesRegistry)
{
    for (auto iterator = _input.begin (); iterator != _input.end (); ++iterator)
    {
        if (!iterator->first.IsScalar ())
        {
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered map pair with non-scalar key!");
            return false;
        }

        switch (iterator->second.Type ())
        {
        case YAML::NodeType::Undefined:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered undefined node during iteration!");
            return false;

        case YAML::NodeType::Null:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered null node during iteration!");
            return false;

        case YAML::NodeType::Scalar:
        {
            const StandardLayout::Field field =
                FindFieldByName (_mapping, Memory::UniqueString {iterator->first.Scalar ().c_str ()});

            if (!field)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Mapping \"", _mapping.GetName (),
                               "\" does not contain field \"", iterator->first.Scalar ().c_str (), "\"!");
                return false;
            }

            if (!DeserializeLeafValueFromYaml (iterator->second, field.GetValue (_objectAddress), field))
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to deserialize value of field \"", *field.GetName (),
                               "\" from mapping \"", _mapping.GetName (), "\"!");
                return false;
            }

            break;
        }

        case YAML::NodeType::Sequence:
        {
            const StandardLayout::Field field =
                FindFieldByName (_mapping, Memory::UniqueString {iterator->first.Scalar ().c_str ()});

            if (!field)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Mapping \"", _mapping.GetName (),
                               "\" does not contain field \"", *iterator->first.Scalar ().c_str (), "\"!");
                return false;
            }

            if (field.GetArchetype () != StandardLayout::FieldArchetype::VECTOR)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered sequence value for non-vector field \"",
                               *field.GetName (), "\" of mapping \"", *_mapping.GetName (), "\"!");
                return false;
            }

            const YAML::Node &sequenceNode = iterator->second;
            void *vectorAddress = field.GetValue (_objectAddress);
            const auto vectorSize = static_cast<std::uint32_t> (sequenceNode.size ());
            const StandardLayout::Mapping &itemMapping = field.GetVectorItemMapping ();
            Container::UntypedVectorUtility::InitSize (vectorAddress, vectorSize * itemMapping.GetObjectSize ());
            int32_t itemIndex = 0u;

            for (std::uint8_t *pointer = Container::UntypedVectorUtility::Begin (vectorAddress);
                 pointer != Container::UntypedVectorUtility::End (vectorAddress);
                 pointer += itemMapping.GetObjectSize ())
            {
                itemMapping.Construct (pointer);
                if (!DeserializeObjectFromYaml (sequenceNode[itemIndex], pointer, itemMapping, _patchableTypesRegistry))
                {
                    return false;
                }

                ++itemIndex;
            }

            break;
        }

        case YAML::NodeType::Map:
        {
            const StandardLayout::Field field =
                FindFieldByName (_mapping, Memory::UniqueString {iterator->first.Scalar ().c_str ()});

            if (!field)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Mapping \"", _mapping.GetName (),
                               "\" does not contain field \"", *iterator->first.Scalar ().c_str (), "\"!");
                return false;
            }

            switch (field.GetArchetype ())
            {
            case StandardLayout::FieldArchetype::BIT:
            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::STRING:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::UNIQUE_STRING:
            case StandardLayout::FieldArchetype::UTF8_STRING:
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered map value for elementary field \"",
                               *field.GetName (), "\" of mapping \"", *_mapping.GetName (), "\"!");
                return false;

            case StandardLayout::FieldArchetype::NESTED_OBJECT:
                if (!DeserializeObjectFromYaml (iterator->second, field.GetValue (_objectAddress),
                                                field.GetNestedObjectMapping (), _patchableTypesRegistry))
                {
                    return false;
                }

                break;

            case StandardLayout::FieldArchetype::VECTOR:
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered map value for vector field \"",
                               *field.GetName (), "\" of mapping \"", *_mapping.GetName (), "\"!");
                return false;

            case StandardLayout::FieldArchetype::PATCH:
                YAML::Node typeNode = (iterator->second)["type"];

                if (!typeNode.IsScalar ())
                {
                    EMERGENCE_LOG (ERROR, "Serialization::Yaml: Patch type node is not a scalar!");
                    return false;
                }

                YAML::Node contentNode = (iterator->second)["content"];
                if (!contentNode.IsMap ())
                {
                    EMERGENCE_LOG (ERROR, "Serialization::Yaml: Patch content node is not a map!");
                    return false;
                }

                StandardLayout::Mapping patchMapping =
                    _patchableTypesRegistry.Get (Memory::UniqueString {typeNode.Scalar ().c_str ()});

                if (!patchMapping)
                {
                    EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to find patchable type \"",
                                   typeNode.Scalar ().c_str (), "\" requested by patch field!");
                    return false;
                }

                StandardLayout::PatchBuilder builder;
                builder.Begin (patchMapping);

                if (!DeserializePatchBodyFromYaml (contentNode, patchMapping, builder, ""))
                {
                    return false;
                }

                *static_cast<StandardLayout::Patch *> (field.GetValue (_objectAddress)) = builder.End ();
                break;
            }

            break;
        }
        }
    }

    return true;
}

void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    YAML::Node node {YAML::NodeType::Map};
    SerializeObjectToYaml (node, _object, _mapping);
    _output << node;
}

bool DeserializeObject (std::istream &_input,
                        void *_object,
                        const StandardLayout::Mapping &_mapping,
                        const Container::MappingRegistry &_patchableTypesRegistry) noexcept
{
    YAML::Node node = YAML::Load (_input);
    if (!node.IsMap ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to parse YAML node from given input!");
        return false;
    }

    return DeserializeObjectFromYaml (node, _object, _mapping, _patchableTypesRegistry);
}
} // namespace Emergence::Serialization::Yaml
