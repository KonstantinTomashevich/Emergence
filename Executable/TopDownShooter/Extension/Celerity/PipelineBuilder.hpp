#pragma once

#include <unordered_set>

#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Flow/TaskRegister.hpp>

#include <Task/Executor.hpp>

namespace Emergence::Celerity
{
class TaskConstructor final
{
public:
    TaskConstructor (const TaskConstructor &_other) = delete;

    TaskConstructor (TaskConstructor &&_other) noexcept;

    ~TaskConstructor () noexcept;

    void DependOn (const char *_taskOrCheckpoint) noexcept;

    void MakeDependencyOf (const char *_taskOrCheckpoint) noexcept;

    [[nodiscard]] Warehouse::FetchSingletonQuery FetchSingleton (const StandardLayout::Mapping &_typeMapping);

    [[nodiscard]] Warehouse::ModifySingletonQuery ModifySingleton (
        const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::InsertShortTermQuery InsertShortTerm (
        const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::FetchSequenceQuery FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::ModifySequenceQuery ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::InsertLongTermQuery InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    [[nodiscard]] Warehouse::FetchValueQuery FetchValue (
        const StandardLayout::Mapping &_typeMapping, const std::vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] Warehouse::ModifyValueQuery ModifyValue (
        const StandardLayout::Mapping &_typeMapping, const std::vector<StandardLayout::FieldId> &_keyFields) noexcept;

    [[nodiscard]] Warehouse::FetchAscendingRangeQuery FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                                           StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::ModifyAscendingRangeQuery ModifyAscendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::FetchDescendingRangeQuery FetchDescendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::ModifyDescendingRangeQuery ModifyDescendingRange (
        const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept;

    [[nodiscard]] Warehouse::FetchShapeIntersectionQuery FetchShapeIntersection (
        const StandardLayout::Mapping &_typeMapping, const std::vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const StandardLayout::Mapping &_typeMapping, const std::vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::FetchRayIntersectionQuery FetchRayIntersection (
        const StandardLayout::Mapping &_typeMapping, const std::vector<Warehouse::Dimension> &_dimensions) noexcept;

    [[nodiscard]] Warehouse::ModifyRayIntersectionQuery ModifyRayIntersection (
        const StandardLayout::Mapping &_typeMapping, const std::vector<Warehouse::Dimension> &_dimensions) noexcept;

    void SetExecutor (std::function<void ()> _executor) noexcept;

    [[nodiscard]] World *GetWorld () const noexcept;

    TaskConstructor &operator= (const TaskConstructor &_other) = delete;

    /// Move-assignment is allowed, because it makes construction of several tasks from one function easier.
    TaskConstructor &operator= (TaskConstructor &&_other) noexcept;

private:
    friend class PipelineBuilder;

    TaskConstructor (PipelineBuilder *_parent, const char *_name) noexcept;

    PipelineBuilder *parent;
    Flow::Task task;
};

class PipelineBuilder final
{
public:
    explicit PipelineBuilder (World *_targetWorld) noexcept;

    PipelineBuilder (const PipelineBuilder &_other) = delete;

    PipelineBuilder (PipelineBuilder &&_other) = delete;

    ~PipelineBuilder () = default;

    void Begin () noexcept;

    [[nodiscard]] TaskConstructor AddTask (const char *_name) noexcept;

    void AddCheckpoint (const char *_name) noexcept;

    [[nodiscard]] Pipeline *End (std::size_t _maximumChildThreads) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PipelineBuilder);

private:
    friend class TaskConstructor;

    void FinishTaskRegistration (Flow::Task _task) noexcept;

    World *world;
    Flow::TaskRegister taskRegister;
    std::unordered_set<std::string> registeredResources;
};
} // namespace Emergence::Celerity
