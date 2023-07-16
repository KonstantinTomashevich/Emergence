#include <Celerity/Asset/Events.hpp>

#include <Container/StringBuilder.hpp>

namespace Emergence::Celerity
{
using namespace Memory::Literals;

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (AssetRemovedNormalEvent, id);

AssetReferenceBindingEventMap RegisterAssetEvents (EventRegistrar &_registrar,
                                                   const AssetReferenceBindingList &_bindings) noexcept
{
    _registrar.OnRemoveEvent ({{AssetRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                               Asset::Reflect ().mapping,
                               {{Asset::Reflect ().id, AssetRemovedNormalEvent::Reflect ().id}}});

    StandardLayout::MappingBuilder builder;
    AssetReferenceBindingEventMap eventMap;

    Container::Vector<StandardLayout::FieldId> assetFields {GetAssetBindingAllocationGroup ()};
    Container::Vector<CopyOutField> assetCopyOut {GetAssetBindingAllocationGroup ()};
    Container::Vector<CopyOutField> unchangedCopyOut {GetAssetBindingAllocationGroup ()};
    Container::Vector<CopyOutField> changedCopyOut {GetAssetBindingAllocationGroup ()};

    for (const AssetReferenceBinding &binding : _bindings)
    {
        auto generateAssetReferenceList =
            [&builder, &binding] (std::size_t _listOffset, Container::Vector<StandardLayout::FieldId> &_trackedFields,
                                  Container::Vector<CopyOutField> &_copyOut)
        {
            _trackedFields.reserve (binding.references.size ());
            _copyOut.reserve (binding.references.size ());

            for (std::size_t assetIndex = 0u; assetIndex < binding.references.size (); ++assetIndex)
            {
                const StandardLayout::FieldId eventField = builder.RegisterUniqueString (
                    binding.objectType.GetField (binding.references[assetIndex].field).GetName (),
                    _listOffset + sizeof (Memory::UniqueString) * assetIndex);

                _trackedFields.emplace_back (binding.references[assetIndex].field);
                _copyOut.emplace_back () = {binding.references[assetIndex].field, eventField};
            }
        };

        AssetReferenceBindingHookEvents &hook = eventMap.hooks[binding.objectType];
        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "AddedNormalEvent")},
                       sizeof (AssetUserAddedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserAddedEventView));

        assetFields.clear ();
        assetCopyOut.clear ();
        generateAssetReferenceList (offsetof (AssetUserAddedEventView, assetReferences), assetFields, assetCopyOut);
        hook.onObjectAddedNormal = builder.End ();
        _registrar.OnAddEvent ({{hook.onObjectAddedNormal, EventRoute::NORMAL}, binding.objectType, assetCopyOut});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "AddedFixedToNormalEvent")},
                       sizeof (AssetUserAddedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserAddedEventView));

        assetFields.clear ();
        assetCopyOut.clear ();
        generateAssetReferenceList (offsetof (AssetUserAddedEventView, assetReferences), assetFields, assetCopyOut);
        hook.onObjectAddedFixedToNormal = builder.End ();
        _registrar.OnAddEvent (
            {{hook.onObjectAddedFixedToNormal, EventRoute::FROM_FIXED_TO_NORMAL}, binding.objectType, assetCopyOut});

        auto generatedChangedAssetReferenceList =
            [&assetFields, &unchangedCopyOut, &changedCopyOut, &binding, &builder] ()
        {
            assetFields.clear ();
            unchangedCopyOut.clear ();
            changedCopyOut.clear ();

            for (std::size_t assetIndex = 0u; assetIndex < binding.references.size (); ++assetIndex)
            {
                const StandardLayout::FieldId oldField = builder.RegisterUniqueString (
                    Memory::UniqueString {EMERGENCE_BUILD_STRING (
                        binding.objectType.GetField (binding.references[assetIndex].field).GetName (), "Old")},
                    offsetof (AssetUserChangedEventView, assetReferenceSequence) +
                        sizeof (Memory::UniqueString) * assetIndex);

                const StandardLayout::FieldId newField = builder.RegisterUniqueString (
                    Memory::UniqueString {EMERGENCE_BUILD_STRING (
                        binding.objectType.GetField (binding.references[assetIndex].field).GetName (), "New")},
                    offsetof (AssetUserChangedEventView, assetReferenceSequence) +
                        sizeof (Memory::UniqueString) * (binding.references.size () + assetIndex));

                assetFields.emplace_back (binding.references[assetIndex].field);
                unchangedCopyOut.emplace_back () = {binding.references[assetIndex].field, oldField};
                changedCopyOut.emplace_back () = {binding.references[assetIndex].field, newField};
            }
        };

        builder.Begin (
            Memory::UniqueString {
                EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (), "ChangedNormalEvent")},
            sizeof (AssetUserChangedEventView) + sizeof (Memory::UniqueString) * binding.references.size () * 2u,
            alignof (AssetUserChangedEventView));

        generatedChangedAssetReferenceList ();
        hook.onAnyReferenceChangedNormal = builder.End ();
        _registrar.OnChangeEvent ({{hook.onAnyReferenceChangedNormal, EventRoute::NORMAL},
                                   binding.objectType,
                                   assetFields,
                                   unchangedCopyOut,
                                   changedCopyOut});

        builder.Begin (
            Memory::UniqueString {
                EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (), "ChangedFixedToNormalEvent")},
            sizeof (AssetUserChangedEventView) + sizeof (Memory::UniqueString) * binding.references.size () * 2u,
            alignof (AssetUserChangedEventView));

        generatedChangedAssetReferenceList ();
        hook.onAnyReferenceChangedFixedToNormal = builder.End ();
        _registrar.OnChangeEvent ({{hook.onAnyReferenceChangedFixedToNormal, EventRoute::FROM_FIXED_TO_NORMAL},
                                   binding.objectType,
                                   assetFields,
                                   unchangedCopyOut,
                                   changedCopyOut});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "RemovedNormalEvent")},
                       sizeof (AssetUserRemovedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserRemovedEventView));

        assetFields.clear ();
        assetCopyOut.clear ();
        generateAssetReferenceList (offsetof (AssetUserRemovedEventView, assetReferences), assetFields, assetCopyOut);
        hook.onObjectRemovedNormal = builder.End ();
        _registrar.OnRemoveEvent ({{hook.onObjectRemovedNormal, EventRoute::NORMAL}, binding.objectType, assetCopyOut});

        builder.Begin (Memory::UniqueString {EMERGENCE_BUILD_STRING ("AssetUser", binding.objectType.GetName (),
                                                                     "RemovedFixedToNormalEvent")},
                       sizeof (AssetUserRemovedEventView) + sizeof (Memory::UniqueString) * binding.references.size (),
                       alignof (AssetUserRemovedEventView));

        assetFields.clear ();
        assetCopyOut.clear ();
        generateAssetReferenceList (offsetof (AssetUserRemovedEventView, assetReferences), assetFields, assetCopyOut);
        hook.onObjectRemovedFixedToNormal = builder.End ();
        _registrar.OnRemoveEvent (
            {{hook.onObjectRemovedFixedToNormal, EventRoute::FROM_FIXED_TO_NORMAL}, binding.objectType, assetCopyOut});

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
