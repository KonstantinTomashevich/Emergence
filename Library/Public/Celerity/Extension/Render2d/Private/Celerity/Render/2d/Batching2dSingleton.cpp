#include <Celerity/Render/2d/Batching2dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const Batching2dSingleton::Reflection &Batching2dSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Batching2dSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

Batch2d &Batching2dSingleton::GetBatch (std::size_t _viewportIndex,
                                        std::uint16_t _layer,
                                        Memory::UniqueString _materialInstanceId) noexcept
{
    EMERGENCE_ASSERT (_viewportIndex < viewports.size ());
    ViewportInfoContainer &viewport = viewports[_viewportIndex];

    auto next = std::upper_bound (viewport.batches.begin (), viewport.batches.end (),
                                  std::make_pair (_layer, _materialInstanceId),
                                  [] (const auto &_query, const Batch2d &_batch)
                                  {
                                      if (_query.first != _batch.layer)
                                      {
                                          return _query.first < _batch.layer;
                                      }

                                      return *_query.second < *_batch.materialInstanceId;
                                  });

    if (next != viewport.batches.begin ())
    {
        auto previous = next - 1u;
        if (previous->layer == _layer && previous->materialInstanceId == _materialInstanceId)
        {
            return *previous;
        }
    }

    if (freeBatches.empty ())
    {
        return *viewport.batches.emplace (
            next, Batch2d {_layer, _materialInstanceId, Container::Vector<UniqueId> {viewport.batches.get_allocator ()},
                           Container::Vector<UniqueId> {viewport.batches.get_allocator ()}});
    }

    Batch2d &pooledBatch = freeBatches.back ();
    freeBatches.pop_back ();

    pooledBatch.layer = _layer;
    pooledBatch.materialInstanceId = _materialInstanceId;
    return *viewport.batches.emplace (next, std::move (pooledBatch));
}

void Batching2dSingleton::Reset () noexcept
{
    std::size_t viewportBatchCount = 0u;
    for (ViewportInfoContainer &viewport : viewports)
    {
        viewportBatchCount += viewport.batches.size ();
    }

    freeBatches.reserve (freeBatches.size () + viewportBatchCount);
    for (ViewportInfoContainer &viewport : viewports)
    {
        for (Batch2d &batch : viewport.batches)
        {
            freeBatches.emplace_back (std::move (batch));
        }

        viewport.batches.clear ();
    }

    viewports.clear ();
}
} // namespace Emergence::Celerity
