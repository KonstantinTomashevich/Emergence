#include <Celerity/Query/InsertLongTermQuery.hpp>

namespace Emergence::Celerity
{
InsertLongTermQuery::Cursor::Cursor (InsertLongTermQuery::Cursor &&_other) noexcept
    : source (std::move (_other.source)),
      eventsOnAdd (_other.eventsOnAdd),
      current (_other.current)
{
    _other.eventsOnAdd = nullptr;
    _other.current = nullptr;
}

InsertLongTermQuery::Cursor::~Cursor () noexcept
{
    FireEvent ();
}

void *InsertLongTermQuery::Cursor::operator++ () noexcept
{
    FireEvent ();
    current = ++source;
    return current;
}

InsertLongTermQuery::Cursor::Cursor (Warehouse::InsertLongTermQuery::Cursor _source,
                                     TrivialEventTriggerInstanceRow *_eventsOnAdd) noexcept
    : source (std::move (_source)),
      eventsOnAdd (_eventsOnAdd)
{
}

void InsertLongTermQuery::Cursor::FireEvent () noexcept
{
    if (eventsOnAdd && current)
    {
        for (TrivialEventTriggerInstance &trigger : *eventsOnAdd)
        {
            trigger.Trigger (current);
        }
    }
}

InsertLongTermQuery::Cursor InsertLongTermQuery::Execute () noexcept
{
    return Cursor {source.Execute (), eventsOnAdd};
}

InsertLongTermQuery::~InsertLongTermQuery () noexcept = default;

InsertLongTermQuery::InsertLongTermQuery (const InsertLongTermQuery &_other) noexcept = default;

InsertLongTermQuery::InsertLongTermQuery (InsertLongTermQuery &&_other) noexcept = default;

StandardLayout::Mapping InsertLongTermQuery::GetTypeMapping () const noexcept
{
    return source.GetTypeMapping ();
}

bool InsertLongTermQuery::IsFromRegistry (const Warehouse::Registry &_registry) const noexcept
{
    return source.IsFromRegistry (_registry);
}

void InsertLongTermQuery::AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept
{
    source.AddCustomVisualization (_graph);
}

InsertLongTermQuery::InsertLongTermQuery (Warehouse::InsertLongTermQuery _source,
                                          TrivialEventTriggerInstanceRow *_eventsOnAdd) noexcept
    : source (std::move (_source)),
      eventsOnAdd (_eventsOnAdd)
{
}
} // namespace Emergence::Celerity
