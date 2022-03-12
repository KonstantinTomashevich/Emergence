#include <Celerity/Query/ModifySingletonQuery.hpp>

namespace Emergence::Celerity
{
ModifySingletonQuery::Cursor::Cursor (ModifySingletonQuery::Cursor &&_other) noexcept
    : source (std::move (_other.source)),
      changeTracker (_other.changeTracker)
{
    _other.changeTracker = nullptr;
}

ModifySingletonQuery::Cursor::~Cursor () noexcept
{
    if (changeTracker)
    {
        if (void *singleton = *source)
        {
            changeTracker->EndEdition (singleton);
        }
    }
}

void *ModifySingletonQuery::Cursor::operator* () const noexcept
{
    return *source;
}

ModifySingletonQuery::Cursor::Cursor (Warehouse::ModifySingletonQuery::Cursor _source,
                                      ChangeTracker *_changeTracker) noexcept
    : source (std::move (_source)),
      changeTracker (_changeTracker)
{
    if (changeTracker)
    {
        if (void *singleton = *source)
        {
            changeTracker->BeginEdition (singleton);
        }
    }
}

ModifySingletonQuery::ModifySingletonQuery (const ModifySingletonQuery &_other) noexcept = default;

ModifySingletonQuery::ModifySingletonQuery (ModifySingletonQuery &&_other) noexcept = default;

ModifySingletonQuery::~ModifySingletonQuery () noexcept = default;

ModifySingletonQuery::Cursor ModifySingletonQuery::Execute () noexcept
{
    return Cursor (source.Execute (), changeTracker);
}

StandardLayout::Mapping ModifySingletonQuery::GetTypeMapping () const noexcept
{
    return source.GetTypeMapping ();
}

void ModifySingletonQuery::AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept
{
    source.AddCustomVisualization (_graph);
}

ModifySingletonQuery::ModifySingletonQuery (Warehouse::ModifySingletonQuery _source,
                                            ChangeTracker *_changeTracker) noexcept
    : source (std::move (_source)),
      changeTracker (_changeTracker)
{
}
} // namespace Emergence::Celerity
