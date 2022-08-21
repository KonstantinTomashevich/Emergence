#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Log/Log.hpp>

#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/BlockCast.hpp>

// We're linking to static library, but define is not passed to us for some reason.
// Therefore, we need to add it manually.
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

namespace Emergence::Serialization::Yaml
{
static YAML::Node SerializeLeafValueToYaml (const void *_address, const StandardLayout::Field &_field)
{
    YAML::Node result {YAML::NodeType::Scalar};
    switch (_field.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:

        // We are casting to bool to signal that this value must be serialized as boolean.
        result = static_cast<bool> (*static_cast<const uint8_t *> (_address) & (1u << _field.GetBitOffset ()));
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
            result = static_cast<uint16_t> (*static_cast<const uint8_t *> (_address));
            break;

        case 2u:
            result = *static_cast<const uint16_t *> (_address);
            break;

        case 4u:
            result = *static_cast<const uint32_t *> (_address);
            break;

        case 8u:
            result = *static_cast<const uint64_t *> (_address);
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
        result = **static_cast<const Memory::UniqueString *> (_address);
        break;

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
        // Only leaf values are supported.
        assert (false);
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
                *static_cast<uint8_t *> (_address) |= 1u << _field.GetBitOffset ();
            }
            else
            {
                *static_cast<uint8_t *> (_address) &= ~(1u << _field.GetBitOffset ());
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
                *static_cast<uint8_t *> (_address) = _input.as<uint8_t> ();
                break;
            case 2u:
                *static_cast<uint16_t *> (_address) = _input.as<uint16_t> ();
                break;
            case 4u:
                *static_cast<uint32_t *> (_address) = _input.as<uint32_t> ();
                break;
            case 8u:
                *static_cast<uint64_t *> (_address) = _input.as<uint64_t> ();
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
            assert (binary.data ());
            memcpy (_address, binary.data (), std::min (binary.size (), _field.GetSize ()));
            break;
        }

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            *static_cast<Memory::UniqueString *> (_address) = Memory::UniqueString {_input.Scalar ().c_str ()};
            break;

        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // Only leaf values are supported.
            assert (false);
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
                _builder.SetUInt8 (fieldId, _input.as<uint8_t> ());
                break;
            case 2u:
                _builder.SetUInt16 (fieldId, _input.as<uint16_t> ());
                break;
            case 4u:
                _builder.SetUInt32 (fieldId, _input.as<uint32_t> ());
                break;
            case 8u:
                _builder.SetUInt64 (fieldId, _input.as<uint64_t> ());
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
            // Unsupported for patches.
            assert (false);
            break;
        }
    }
    catch ([[maybe_unused]] YAML::Exception &exception)
    {
        return false;
    }

    return true;
}

template <typename LeafDeserializer>
static bool DeserializeFromYaml (const YAML::Node &_input,
                                 LeafDeserializer &_deserializer,
                                 const Container::String &_prefix,
                                 FieldNameLookupCache &_cache)
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
            StandardLayout::Field field = _cache.Lookup (fieldName);

            if (!field)
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Mapping \"", _cache.GetTypeMapping ().GetName (),
                               "\" does not contain field \"", *fieldName, "\"!");
                return false;
            }

            if (!_deserializer (iterator->second, field))
            {
                EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to deserialize value of field \"", *fieldName,
                               "\" from mapping \"", _cache.GetTypeMapping ().GetName (), "\"!");
                return false;
            }

            break;
        }

        case YAML::NodeType::Sequence:
            EMERGENCE_LOG (ERROR, "Serialization::Yaml: Encountered sequence node during iteration!");
            return false;

        case YAML::NodeType::Map:
        {
            if (!DeserializeFromYaml (
                    iterator->second, _deserializer,
                    // NOLINTNEXTLINE(readability-redundant-string-cstr): It's not, because allocators are different.
                    _prefix + iterator->first.Scalar ().c_str () + StandardLayout::PROJECTION_NAME_SEPARATOR, _cache))
            {
                return false;
            }

            break;
        }
        }
    }

    return true;
}

bool DeserializePatchFromYaml (const YAML::Node &_node,
                               StandardLayout::PatchBuilder &_builder,
                               FieldNameLookupCache &_cache)
{
    _builder.Begin (_cache.GetTypeMapping ());
    auto leafDeserializer = [&_builder, &_cache] (const YAML::Node &_input, const StandardLayout::Field &_field)
    {
        return DeserializePatchLeafValueFromYaml (_input, _builder, _cache.GetTypeMapping (), _field);
    };

    return DeserializeFromYaml (_node, leafDeserializer, "", _cache);
}

void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    YAML::Node node {YAML::NodeType::Map};
    SerializeObjectToYaml (node, _object, _mapping);
    _output << node;
}

bool DeserializeObject (std::istream &_input, void *_object, FieldNameLookupCache &_cache) noexcept
{
    YAML::Node node = YAML::Load (_input);
    if (!node.IsMap ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to parse YAML node from given input!");
        return false;
    }

    auto leafDeserializer = [_object] (const YAML::Node &_input, const StandardLayout::Field &_field)
    {
        return DeserializeLeafValueFromYaml (_input, _field.GetValue (_object), _field);
    };

    return DeserializeFromYaml (node, leafDeserializer, "", _cache);
}

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept
{
    YAML::Node root {YAML::NodeType::Map};
    for (const StandardLayout::Patch::ChangeInfo &change : _patch)
    {
        StandardLayout::Field field = _patch.GetTypeMapping ().GetField (change.field);
        SerializePatchValue (field, change.newValue, root, *field.GetName ());
    }

    _output << root;
}

bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       FieldNameLookupCache &_cache) noexcept
{
    YAML::Node node = YAML::Load (_input);
    if (!node.IsMap ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to parse YAML node from given input!");
        return false;
    }

    return DeserializePatchFromYaml (node, _builder, _cache);
}

static_assert (sizeof (YamlRootPlaceholder) == sizeof (YAML::Node));

static_assert (sizeof (YamlIteratorPlaceholder) == sizeof (YAML::Node::iterator));

BundleDeserializerBase::BundleDeserializerBase () noexcept
{
    auto *node = new (yamlRootPlaceholder.data ()) YAML::Node {};
    new (yamlIteratorPlaceholder.data ()) YAML::Node::iterator (node->begin ());
}

BundleDeserializerBase::~BundleDeserializerBase () noexcept
{
    block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder).~iterator_base ();
    block_cast<YAML::Node> (yamlRootPlaceholder).~Node ();
}

bool BundleDeserializerBase::Begin (std::istream &_input) noexcept
{
    auto &root = block_cast<YAML::Node> (yamlRootPlaceholder);
    root = YAML::Load (_input);

    if (!root.IsSequence ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to parse YAML node from given input!");
        return false;
    }

    block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder) = root.begin ();
    return true;
}

bool BundleDeserializerBase::HasNext () const noexcept
{
    return block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder) !=
           block_cast<YAML::Node> (yamlRootPlaceholder).end ();
}

void BundleDeserializerBase::End () noexcept
{
    auto &root = block_cast<YAML::Node> (yamlRootPlaceholder);
    root = YAML::Node {};
    block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder) = root.begin ();
}

ObjectBundleSerializer::ObjectBundleSerializer (StandardLayout::Mapping _mapping) noexcept
    : mapping (std::move (_mapping))
{
    new (yamlRootPlaceholder.data ()) YAML::Node {};
}

ObjectBundleSerializer::~ObjectBundleSerializer () noexcept
{
    block_cast<YAML::Node> (yamlRootPlaceholder).~Node ();
}

void ObjectBundleSerializer::Begin () noexcept
{
    block_cast<YAML::Node> (yamlRootPlaceholder) = YAML::Node {YAML::NodeType::Sequence};
}

void ObjectBundleSerializer::Next (const void *_object) noexcept
{
    auto &root = block_cast<YAML::Node> (yamlRootPlaceholder);
    YAML::Node item {YAML::NodeType::Map};
    SerializeObjectToYaml (item, _object, mapping);
    root.push_back (item);
}

void ObjectBundleSerializer::End (std::ostream &_output) noexcept
{
    auto &root = block_cast<YAML::Node> (yamlRootPlaceholder);
    _output << root;
    root = YAML::Node {};
}

ObjectBundleDeserializer::ObjectBundleDeserializer (StandardLayout::Mapping _mapping) noexcept
    : fieldNameLookupCache (std::move (_mapping))
{
}

bool ObjectBundleDeserializer::Next (void *_object) noexcept
{
    if (!HasNext ())
    {
        return false;
    }

    auto &iterator = block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder);
    auto leafDeserializer = [_object] (const YAML::Node &_input, const StandardLayout::Field &_field)
    {
        return DeserializeLeafValueFromYaml (_input, _field.GetValue (_object), _field);
    };

    const bool successful = DeserializeFromYaml (*iterator, leafDeserializer, "", fieldNameLookupCache);
    ++iterator;
    return successful;
}

void PatchBundleDeserializer::RegisterType (const StandardLayout::Mapping &_mapping) noexcept
{
    cachesByTypeName.emplace (_mapping.GetName (), FieldNameLookupCache {_mapping});
}

Container::Optional<StandardLayout::Patch> PatchBundleDeserializer::Next () noexcept
{
    if (!HasNext ())
    {
        return std::nullopt;
    }

    auto &iterator = block_cast<YAML::Node::iterator> (yamlIteratorPlaceholder);
    YAML::Node typeNode = (*iterator)["type"];

    if (!typeNode.IsScalar ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Patch type node is not a scalar!");
        return std::nullopt;
    }

    YAML::Node contentNode = (*iterator)["content"];
    if (!contentNode.IsMap ())
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Patch content node is not a map!");
        return std::nullopt;
    }

    FieldNameLookupCache *cache = RequestCache (Memory::UniqueString {typeNode.Scalar ().c_str ()});
    if (!cache)
    {
        EMERGENCE_LOG (ERROR, "Serialization::Yaml: Unable to find type \"", typeNode.Scalar ().c_str (),
                       "\" requested by patch bundle!");
        return std::nullopt;
    }

    StandardLayout::PatchBuilder builder;
    if (DeserializePatchFromYaml (contentNode, builder, *cache))
    {
        ++iterator;
        return builder.End ();
    }

    return std::nullopt;
}

FieldNameLookupCache *PatchBundleDeserializer::RequestCache (Memory::UniqueString _typeName) noexcept
{
    if (auto iterator = cachesByTypeName.find (_typeName); iterator != cachesByTypeName.end ())
    {
        return &iterator->second;
    }

    return nullptr;
}
} // namespace Emergence::Serialization::Yaml
