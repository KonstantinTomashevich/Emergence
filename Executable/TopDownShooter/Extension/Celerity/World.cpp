#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

namespace Emergence::Celerity
{
World::World (std::string _name) noexcept : registry (std::move (_name))
{
}
} // namespace Emergence::Celerity
