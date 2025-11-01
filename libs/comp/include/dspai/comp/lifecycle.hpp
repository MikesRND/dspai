#pragma once

#include <system_error>

namespace dspai::comp {

/**
 * Lifecycle states of a component
 *
 */
enum class LifecycleState { 
        Uninitialized, ///< Default state after construction and before initialization
        Initialized,   ///< State after successful initialize() call
        Terminated     ///< State after terminate() call; resources deallocated, terminal state
};


/**
 * Interface for managing the lifecycle of an object.
 * 
 * Thread Safety: Methods are NOT thread-safe. Caller must provide synchronization.
 * Concurrent access to any methods requires external locking.
 * 
 */
class ILifecycle {
public:


    ILifecycle() = default;                            /// Default constructor
    ILifecycle(const ILifecycle&) = delete;            /// Delete copy constructor
    ILifecycle& operator=(const ILifecycle&) = delete; /// Delete copy assignment
    ILifecycle(ILifecycle&&) = default;                /// Default move constructor
    ILifecycle& operator=(ILifecycle&&) = default;     /// Default move assignment
    virtual ~ILifecycle() noexcept = default;          /// Virtual destructor

    /**
     * Returns the current lifecycle state of the object.
     */
    virtual LifecycleState lifecycle_state() const noexcept = 0;

    /**
     * @brief Initialize component.
     * 
     * - Allocates resources and prepares the component.
     * - No exceptions
     * - Only callable when LifecycleState is Uninitialized.
     * - On success, transitions to Initialized state.
     * - On failure, remains in Uninitialized state.
     * - Strong guarantee: either fully initialized or remains `Uninitialized`.
     * 
     * @return success
     */
    virtual std::error_code initialize() noexcept = 0;

    /**
     * @brief Terminate Component
     * 
     * - Deallocates resources and cleans up. 
     * - Can be called from any state
     * - Idempotent: multiple calls have no additional effect.
     * - Transitions to Terminated state.
     * 
     */
    virtual void terminate() noexcept = 0;
};

} // namespace dspai::comp