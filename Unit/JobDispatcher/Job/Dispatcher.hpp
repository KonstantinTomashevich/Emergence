#pragma once

#include <array>
#include <functional>
#include <thread>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Job
{
// TODO: Right now there is no tests for JobDispatcher, because it is covered by other services.
//       We should add separate tests for this service later.

/// \brief Priority affects how job will be dispatched and scheduled.
enum class Priority
{
    /// \brief Foreground jobs keep the application alive, therefore there is
    ///        reserved sub-pool of threads that only execute foreground jobs,
    FOREGROUND = 0u,

    /// \brief Background jobs are meant to do some heavy work in background, like loading assets.
    /// \details If there are both foreground and background jobs in queue, background job will be
    ///          scheduled first unless background job thread limit is reached. This prevents
    ///          foreground jobs from taking all the resources.
    BACKGROUND
};

/// \brief Dispatches received jobs between worker threads.
/// \details Usage of shared job dispatcher allows to avoid creation of multiple private thread pools.
class Dispatcher final
{
public:
    /// \brief Type of a job, that can be executed by job dispatcher.
    using Job = std::function<void ()>;

    /// \brief Utility class for optimized dispatching multiple jobs at once.
    /// \details It is advised to use this class instead of calling dispatch multiple times if you have
    ///          more than one job to dispatch as this class might provide better dispatch performance.
    class Batch final
    {
    public:
        /// \brief Creates batch for given dispatcher instance.
        Batch (Dispatcher &_dispatcher) noexcept;

        Batch (const Batch &_other) = delete;

        Batch (Batch &&_other) = delete;

        ~Batch () noexcept;

        /// \brief The same as Dispatcher::Dispatch.
        void Dispatch (Priority _jobPriority, Job _job) noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Batch);

    private:
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);
    };

    /// \return Global instance with std::thread::hardware_concurrency number of threads.
    /// \details It is recommended to use global instance instead of creating local dispatchers.
    static Dispatcher &Global () noexcept;

    /// \brief Constructs new dispatcher with thread pool of given size.
    Dispatcher (std::size_t _threadCount) noexcept;

    Dispatcher (const Dispatcher &_other) = delete;

    Dispatcher (Dispatcher &&_other) = delete;

    ~Dispatcher () noexcept;

    /// \brief Dispatches given job.
    /// \details If all worker threads are busy, job will be placed into waiting structure.
    ///          Behaviour of waiting structure is implementation-dependent.
    void Dispatch (Priority _jobPriority, Job _job) noexcept;

    /// \return Count of threads that don't have any job at the moment.
    [[nodiscard]] std::size_t GetAvailableThreadsCount () const noexcept;

    /// \brief Sets maximum count of threads occupied by Priority::BACKGROUND jobs.
    /// \details If there is already more threads that execute background jobs,
    ///          no jobs will be cancelled and maximum will only be applied after
    ///          thread number goes down to it.
    void SetMaximumThreadsForBackgroundExecution (std::size_t _maxBackgroundThreadCount) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Dispatcher);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 18u);
};
} // namespace Emergence::Job
