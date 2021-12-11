#include <Celerity/Pipeline.hpp>

namespace Emergence::Celerity
{
void Pipeline::Execute () noexcept
{
    executor.Execute ();
}

Pipeline::Pipeline (const Task::Collection &_collection, std::size_t _maximumChildThreads)
    : executor (_collection, _maximumChildThreads)
{
}
} // namespace Emergence::Celerity
