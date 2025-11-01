#pragma once

#include <dspai/lifecycle.hpp>
#include <cstdint>

namespace dspai {

/**
 * Execution states of a component
 *
 */
enum class ExecutionState {
    Reset,   ///< Default state after initialization
    Running, ///< The component is actively processing
    Done     ///< Processing is complete
};


/**
 * Execution interface for a Component
 *
 * This interface is only valid when the component is in the Initialized lifecycle state.
 *
 * When LifecycleState != Initialized, the interface
 * should provide the following pre-defined exception-free behavior:
 * - is_ready() returns false
 * - count() returns 0
 * - execute() is a no-op and returns false
 * - reset() is a no-op
 * - execution_state() returns Reset when Uninitialized or Done if Terminated
 *
 */
class IExecution : public ILifecycle {
public:

    /**
     * Returns the current execution state of the object.
     *
     */
    virtual ExecutionState execution_state() const noexcept = 0;

    /**
     * @brief Get the current iteration count
     *
     * Always returns zero after initialize() or reset().
     *
     * @return std::uint64_t: The current iteration count
     */
    virtual std::uint64_t count() const noexcept = 0;

    /**
     * @brief Execute one step of processing
     *
     * - Normally called from Reset or Running states.
     * - Can be called multiple times until processing is complete
     * - Transitions to either Running or Done state
     * - Should not throw exceptions for performance reasons
     * - Increments the iteration count on each successful call to execute()
     * - If called in Done state, it is a no-op and returns false.
     *
     * @return done: true if processing is complete or cannot continue
     */
    virtual bool execute() noexcept = 0;

    /**
     * @brief Reset component
     *
     * - Makes component ready for new processing
     * - Reset internal states and variables to default values
     * - Reset iteration count to zero
     * - Can be called from Reset, Running, or Done state.
     * - When called from Reset, it is a no-op
     * - Transitions execution state to Reset
     * - Idempotent: multiple calls are no-ops after the first
     * - Should not perform any allocations or deallocations
     */
    virtual void reset() noexcept = 0;

    /**
     * @brief Check if component is ready for execution
     *
     * @return ready: true if Initialized and ExecutionState is Reset or Running.
     */
    bool is_ready() const noexcept {
        if (lifecycle_state() != LifecycleState::Initialized) return false;
        auto s = execution_state();
        return s == ExecutionState::Reset || s == ExecutionState::Running;
    }


};

} // namespace dspai