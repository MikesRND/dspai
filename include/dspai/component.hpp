#pragma once

#include "execution.hpp"

namespace dspai {

/**
 * Base component class 
 *
 * Implements IExecution and ILifecycle interfaces
 * - Provides state management
 * - Enforces lifecycle/execution state transitions.
 * - VMI: derived classes should override do* methods to implement specific behavior.
 *
 * Thread Safety: NOT thread-safe. External synchronization required.
 */
class Component : public IExecution {
public:
    Component() = default;
    virtual ~Component() noexcept = default;

    // ILifecycle interface
    LifecycleState lifecycle_state() const noexcept override {
        return lifecycle_state_;
    }

    std::error_code initialize() noexcept override {
        if (lifecycle_state_ != LifecycleState::Uninitialized) {
            return std::make_error_code(std::errc::operation_not_permitted);
        }

        auto result = doInitialize();
        if (!result) {
            lifecycle_state_ = LifecycleState::Initialized;
            execution_state_ = ExecutionState::Reset;
            count_ = 0;
        }
        return result;
    }

    void terminate() noexcept override {
        if (lifecycle_state_ == LifecycleState::Terminated) {
            return; // Idempotent
        }

        doTerminate();
        lifecycle_state_ = LifecycleState::Terminated;
        execution_state_ = ExecutionState::Done;
        count_ = 0;
    }

    // IExecution interface
    ExecutionState execution_state() const noexcept override {
        if (lifecycle_state_ == LifecycleState::Uninitialized) {
            return ExecutionState::Reset;
        }
        if (lifecycle_state_ == LifecycleState::Terminated) {
            return ExecutionState::Done;
        }
        return execution_state_;
    }

    std::uint64_t count() const noexcept override {
        if (lifecycle_state_ != LifecycleState::Initialized) {
            return 0;
        }
        return count_;
    }

    bool execute() noexcept override {
        if (lifecycle_state_ != LifecycleState::Initialized) {
            return false; // No-op when not initialized
        }

        if (execution_state_ == ExecutionState::Done) {
            return true; // Already done
        }

        // Transition to Running if in Reset state
        if (execution_state_ == ExecutionState::Reset) {
            execution_state_ = ExecutionState::Running;
        }

        // Execute the actual work
        bool done = doExecute();
        count_++;

        if (done) {
            execution_state_ = ExecutionState::Done;
        }

        return done;
    }

    void reset() noexcept override {
        if (lifecycle_state_ != LifecycleState::Initialized) {
            return; // No-op when not initialized
        }

        if (execution_state_ == ExecutionState::Reset) {
            return; // Already in reset state (idempotent)
        }

        doReset();
        execution_state_ = ExecutionState::Reset;
        count_ = 0;
    }

protected:
    /**
     * Override to implement initialization logic.
     * - All or nothing initialization. 
     * - No persistent side effects on failure.
     *
     * @return std::error_code - empty on success, error code on failure
     */
    virtual std::error_code doInitialize() noexcept = 0;

    /**
     * Override to implement reset logic.
     * 
     * - Reset all internal state machines and variables to their default values
     * - Re-compute all derived states from scratch
     * - No allocations or deallocations
     * 
     */
    virtual void doReset() noexcept = 0;

    /**
     * Override to implement compute logic.
     * 
     * - Perform processing
     * - No exceptions
     * - No dynamic allocations
     *
     * @return true if processing is complete, false to continue
     */
    virtual bool doExecute() noexcept = 0;

    /**
     * Override to implement termination logic.
     * - Cleanup resources - best effort
     * - No exceptions
     */
    virtual void doTerminate() noexcept = 0;



private:
    LifecycleState lifecycle_state_ = LifecycleState::Uninitialized;
    ExecutionState execution_state_ = ExecutionState::Reset;
    std::uint64_t count_ = 0;
};

} // namespace dspai