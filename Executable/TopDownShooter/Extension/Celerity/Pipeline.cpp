#include <Celerity/Pipeline.hpp>

namespace Emergence::Celerity
{
Memory::UniqueString Pipeline::GetId () const noexcept
{
    return id;
}

void Pipeline::Execute () noexcept
{
    executor.Execute ();
}

Pipeline::Pipeline (Memory::UniqueString _id, const Task::Collection &_collection, std::size_t _maximumChildThreads)
    : id (_id),
      executor (_collection, _maximumChildThreads)
{
}
} // namespace Emergence::Celerity
