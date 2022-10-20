#include <Celerity/Asset/Events.hpp>

#include <Container/StringBuilder.hpp>

namespace Emergence::Celerity
{
using namespace Memory::Literals;

AssetReferenceBindingEventMap RegisterAssetEvents (EventRegistrar &_registrar,
                                                   const AssetReferenceBindingList &_bindings) noexcept
{
    StandardLayout::MappingBuilder builder;
    AssetReferenceBindingEventMap eventMap;

    for (const AssetReferenceBinding &binding : _bindings)
    {
        AssetReferenceBindingHookEvents &hook = eventMap.hooks[binding.objectType];
        static_assert (std::is_same_v<uint64_t, UniqueId>);

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "AddedNormalEvent")},
                       sizeof (AssetUserAddedEventView), alignof (AssetUserAddedEventView));
        StandardLayout::FieldId assetUserIdField =
            builder.RegisterUInt64 ("assetUserId"_us, offsetof (AssetUserAddedEventView, assetUserId));
        hook.onObjectAdded = builder.End ();

        _registrar.OnAddEvent ({{hook.onObjectAdded, EventRoute::NORMAL},
                                binding.objectType,
                                {{binding.resourceUserIdField, assetUserIdField}}});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "ChangedNormalEvent")},
                       sizeof (AssetUserChangedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserChangedEventView));
        assetUserIdField = builder.RegisterUInt64 ("assetUserId"_us, offsetof (AssetUserAddedEventView, assetUserId));

        auto generateAssetListTail = [&builder, &binding] (std::size_t _listOffset,
                                                           Container::Vector<StandardLayout::FieldId> &_eventFields,
                                                           Container::Vector<CopyOutField> &_copyOut)
        {
            for (std::size_t assetIndex = 0u; assetIndex < binding.references.size (); ++assetIndex)
            {
                StandardLayout::FieldId eventField = builder.RegisterUniqueString (
                    binding.objectType.GetField (binding.references[assetIndex].field).GetName (),
                    _listOffset + sizeof (Memory::UniqueString) * assetIndex);

                _eventFields.emplace_back (eventField);
                _copyOut.emplace_back () = {binding.references[assetIndex].field, eventField};
            }
        };

        Container::Vector<StandardLayout::FieldId> unchangedAssetsFields {GetAssetBindingAllocationGroup ()};
        Container::Vector<CopyOutField> unchangedAssetsCopyOut {GetAssetBindingAllocationGroup ()};
        generateAssetListTail (offsetof (AssetUserChangedEventView, unchangedAssets), unchangedAssetsFields,
                               unchangedAssetsCopyOut);
        hook.onAnyReferenceChanged = builder.End ();

        _registrar.OnChangeEvent ({{hook.onAnyReferenceChanged, EventRoute::NORMAL},
                                   binding.objectType,
                                   unchangedAssetsFields,
                                   unchangedAssetsCopyOut,
                                   {{binding.resourceUserIdField, assetUserIdField}}});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "RemovedNormalEvent")},
                       sizeof (AssetUserRemovedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserRemovedEventView));

        Container::Vector<StandardLayout::FieldId> assetsFields {GetAssetBindingAllocationGroup ()};
        Container::Vector<CopyOutField> assetsCopyOut {GetAssetBindingAllocationGroup ()};
        generateAssetListTail (offsetof (AssetUserRemovedEventView, assets), assetsFields, assetsCopyOut);

        hook.onObjectRemoved = builder.End ();
        _registrar.OnRemoveEvent ({{hook.onObjectRemoved, EventRoute::NORMAL}, binding.objectType, assetsCopyOut});

        for (const AssetReferenceField &field : binding.references)
        {
            if (!eventMap.stateUpdate.contains (field.assetType))
            {
                builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("Asset", field.assetType.GetName (),
                                                                             "StateUpdateNormalEvent")},
                               sizeof (AssetStateUpdateEventView), alignof (AssetStateUpdateEventView));

                [[maybe_unused]] StandardLayout::FieldId assetIdField =
                    builder.RegisterUniqueString ("assetId"_us, offsetof (AssetStateUpdateEventView, assetId));
                static_assert (std::is_same_v<uint8_t, std::underlying_type_t<AssetState>>);
                [[maybe_unused]] StandardLayout::FieldId stateField =
                    builder.RegisterUInt8 ("state"_us, offsetof (AssetStateUpdateEventView, state));

                StandardLayout::Mapping stateUpdateEvent = builder.End ();
                eventMap.stateUpdate.emplace (field.assetType, stateUpdateEvent);
                _registrar.CustomEvent ({stateUpdateEvent, EventRoute::NORMAL});
            }
        }
    }

    return eventMap;
}
} // namespace Emergence::Celerity
