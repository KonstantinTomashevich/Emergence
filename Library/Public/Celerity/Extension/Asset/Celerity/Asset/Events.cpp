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
        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "AddedNormalEvent")},
                       sizeof (AssetUserAddedEventView), alignof (AssetUserAddedEventView));
        StandardLayout::FieldId assetUserIdField =
            StandardLayout::Registration::RegisterRegularField<decltype (AssetUserAddedEventView::assetUserId)> (
                builder, "assetUserId", offsetof (AssetUserAddedEventView, assetUserId));
        hook.onObjectAdded = builder.End ();

        _registrar.OnAddEvent ({{hook.onObjectAdded, EventRoute::NORMAL},
                                binding.objectType,
                                {{binding.assetUserIdField, assetUserIdField}}});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "ChangedNormalEvent")},
                       sizeof (AssetUserChangedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserChangedEventView));
        
        assetUserIdField =
            StandardLayout::Registration::RegisterRegularField<decltype (AssetUserChangedEventView::assetUserId)> (
                builder, "assetUserId", offsetof (AssetUserChangedEventView, assetUserId));

        auto generateAssetListTail = [&builder, &binding] (std::size_t _listOffset,
                                                           Container::Vector<StandardLayout::FieldId> &_trackedFields,
                                                           Container::Vector<CopyOutField> &_copyOut)
        {
            _trackedFields.reserve (binding.references.size ());
            _copyOut.reserve (binding.references.size ());

            for (std::size_t assetIndex = 0u; assetIndex < binding.references.size (); ++assetIndex)
            {
                StandardLayout::FieldId eventField = builder.RegisterUniqueString (
                    binding.objectType.GetField (binding.references[assetIndex].field).GetName (),
                    _listOffset + sizeof (Memory::UniqueString) * assetIndex);

                _trackedFields.emplace_back (binding.references[assetIndex].field);
                _copyOut.emplace_back () = {binding.references[assetIndex].field, eventField};
            }
        };

        Container::Vector<StandardLayout::FieldId> trackedAssetFields {GetAssetBindingAllocationGroup ()};
        Container::Vector<CopyOutField> unchangedAssetCopyOut {GetAssetBindingAllocationGroup ()};
        generateAssetListTail (offsetof (AssetUserChangedEventView, unchangedAssets), trackedAssetFields,
                               unchangedAssetCopyOut);
        hook.onAnyReferenceChanged = builder.End ();

        _registrar.OnChangeEvent ({{hook.onAnyReferenceChanged, EventRoute::NORMAL},
                                   binding.objectType,
                                   trackedAssetFields,
                                   unchangedAssetCopyOut,
                                   {{binding.assetUserIdField, assetUserIdField}}});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "RemovedNormalEvent")},
                       sizeof (AssetUserRemovedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserRemovedEventView));

        Container::Vector<StandardLayout::FieldId> assetFields {GetAssetBindingAllocationGroup ()};
        Container::Vector<CopyOutField> assetCopyOut {GetAssetBindingAllocationGroup ()};
        generateAssetListTail (offsetof (AssetUserRemovedEventView, assets), assetFields, assetCopyOut);

        hook.onObjectRemoved = builder.End ();
        _registrar.OnRemoveEvent ({{hook.onObjectRemoved, EventRoute::NORMAL}, binding.objectType, assetCopyOut});

        for (const AssetReferenceField &field : binding.references)
        {
            if (!eventMap.stateUpdate.contains (field.assetType))
            {
                builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("Asset", field.assetType.GetName (),
                                                                             "StateUpdateNormalEvent")},
                               sizeof (AssetStateUpdateEventView), alignof (AssetStateUpdateEventView));

                [[maybe_unused]] StandardLayout::FieldId assetIdField =
                    StandardLayout::Registration::RegisterRegularField<decltype (AssetStateUpdateEventView::assetId)> (
                        builder, "assetId", offsetof (AssetStateUpdateEventView, assetId));

                [[maybe_unused]] StandardLayout::FieldId stateField =
                    StandardLayout::Registration::RegisterRegularField<decltype (AssetStateUpdateEventView::state)> (
                        builder, "state", offsetof (AssetStateUpdateEventView, state));

                StandardLayout::Mapping stateUpdateEvent = builder.End ();
                eventMap.stateUpdate.emplace (field.assetType, stateUpdateEvent);
                _registrar.CustomEvent ({stateUpdateEvent, EventRoute::NORMAL});
            }
        }
    }

    return eventMap;
}
} // namespace Emergence::Celerity
