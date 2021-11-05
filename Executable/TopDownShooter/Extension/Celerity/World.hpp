#pragma once

#include <memory>
#include <vector>

#include <Celerity/Pipeline.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Celerity
{
class World final
{
public:
    World (const World &_other) = delete;

    World (World &&_other) = delete;

    ~World () = default;

    explicit World (std::string _name) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (World);

    // TODO: Rules for automatic edition/addition/deletion event firing.

private:
    friend class PipelineBuilder;
    friend class TaskConstructor;

    Warehouse::Registry registry;
    std::vector<std::unique_ptr<Pipeline>> pipelines;
};
} // namespace Emergence::Celerity
