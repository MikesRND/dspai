#include <dspai/comp/component.hpp>
#include <iostream>
#include <cassert>
#include <string>

using namespace dspai::comp;

// Test component implementation
class TestComponent : public Component {
public:
    TestComponent(int max_iterations = 5)
        : max_iterations_(max_iterations), current_iteration_(0) {}

    // Public accessors for testing
    bool initialized() const { return initialized_; }
    bool terminated() const { return terminated_; }
    int current_iteration() const { return current_iteration_; }
    bool reset_called() const { return reset_called_; }

    // Control test behavior
    void set_init_failure(bool fail) { should_fail_init_ = fail; }
    void set_max_iterations(int max) { max_iterations_ = max; }

protected:
    std::error_code doInitialize() noexcept override {
        if (should_fail_init_) {
            return std::make_error_code(std::errc::io_error);
        }
        initialized_ = true;
        current_iteration_ = 0;
        return {};
    }

    void doTerminate() noexcept override {
        terminated_ = true;
        initialized_ = false;
    }

    bool doExecute() noexcept override {
        current_iteration_++;
        return current_iteration_ >= max_iterations_;
    }

    void doReset() noexcept override {
        current_iteration_ = 0;
        reset_called_ = true;
    }

private:
    bool initialized_ = false;
    bool terminated_ = false;
    bool reset_called_ = false;
    bool should_fail_init_ = false;
    int max_iterations_ = 5;
    int current_iteration_ = 0;
};

// Test helper macros
#define TEST(name) void test_##name(); \
    static struct test_##name##_runner { \
        test_##name##_runner() { \
            std::cout << "Running: " #name << "... "; \
            test_##name(); \
            std::cout << "PASSED\n"; \
        } \
    } test_##name##_instance; \
    void test_##name()

#define ASSERT_EQ_ENUM(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "\nAssertion failed: " << #expected << " != " << #actual \
                  << "\n  Expected: " << static_cast<int>(expected) \
                  << "\n  Actual: " << static_cast<int>(actual) \
                  << "\n  At: " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "\nAssertion failed: " << #expected << " != " << #actual \
                  << "\n  Expected: " << (expected) \
                  << "\n  Actual: " << (actual) \
                  << "\n  At: " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "\nAssertion failed: " << #condition \
                  << " is false\n  At: " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        std::cerr << "\nAssertion failed: " << #condition \
                  << " is true\n  At: " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    }

// Test initial state
TEST(initial_state) {
    TestComponent component;
    ASSERT_EQ_ENUM(LifecycleState::Uninitialized, component.lifecycle_state());
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());
    ASSERT_EQ(0u, component.count());
    ASSERT_FALSE(component.is_ready());
}

// Test successful initialization
TEST(initialize_success) {
    TestComponent component;
    auto result = component.initialize();
    ASSERT_FALSE(result); // No error
    ASSERT_EQ_ENUM(LifecycleState::Initialized, component.lifecycle_state());
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());
    ASSERT_EQ(0u, component.count());
    ASSERT_TRUE(component.initialized());
    ASSERT_TRUE(component.is_ready());
}

// Test initialization failure
TEST(initialize_failure) {
    TestComponent component;
    component.set_init_failure(true);
    auto result = component.initialize();
    ASSERT_TRUE(result); // Has error
    ASSERT_EQ_ENUM(LifecycleState::Uninitialized, component.lifecycle_state());
    ASSERT_FALSE(component.initialized());
    ASSERT_FALSE(component.is_ready());
}

// Test double initialization
TEST(double_initialize) {
    TestComponent component;
    auto result1 = component.initialize();
    ASSERT_FALSE(result1);

    auto result2 = component.initialize();
    ASSERT_TRUE(result2); // Should fail - already initialized
    ASSERT_TRUE(result2 == std::errc::operation_not_permitted);
    ASSERT_EQ_ENUM(LifecycleState::Initialized, component.lifecycle_state());
}

// Test execute from uninitialized
TEST(execute_uninitialized) {
    TestComponent component;
    bool done = component.execute();
    ASSERT_FALSE(done);
    ASSERT_EQ(0u, component.count());
    ASSERT_EQ(0, component.current_iteration());
}

// Test normal execution flow
TEST(execute_normal_flow) {
    TestComponent component(3);
    component.initialize();

    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());

    // First execute - should transition to Running
    bool done1 = component.execute();
    ASSERT_FALSE(done1);
    ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());
    ASSERT_EQ(1u, component.count());
    ASSERT_EQ(1, component.current_iteration());

    // Second execute
    bool done2 = component.execute();
    ASSERT_FALSE(done2);
    ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());
    ASSERT_EQ(2u, component.count());
    ASSERT_EQ(2, component.current_iteration());

    // Third execute - should complete
    bool done3 = component.execute();
    ASSERT_TRUE(done3);
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
    ASSERT_EQ(3u, component.count());
    ASSERT_EQ(3, component.current_iteration());
    ASSERT_FALSE(component.is_ready());

    // Execute when done - should return true immediately
    bool done4 = component.execute();
    ASSERT_TRUE(done4);
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
    ASSERT_EQ(3u, component.count()); // Count should not increment
}

// Test reset from various states
TEST(reset_states) {
    TestComponent component(2);

    // Reset when uninitialized - should be no-op
    component.reset();
    ASSERT_FALSE(component.reset_called());

    // Initialize and reset from Reset state
    component.initialize();
    component.reset();
    ASSERT_FALSE(component.reset_called()); // Should be no-op in Reset state

    // Execute once and reset from Running
    component.execute();
    ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());
    component.reset();
    ASSERT_TRUE(component.reset_called());
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());
    ASSERT_EQ(0u, component.count());
    ASSERT_TRUE(component.is_ready());

    // Execute to completion and reset from Done
    component.execute();
    component.execute();
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
    component.reset();
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());
    ASSERT_EQ(0u, component.count());
    ASSERT_TRUE(component.is_ready());
}

// Test terminate from various states
TEST(terminate_states) {
    // Terminate from Uninitialized
    {
        TestComponent component;
        component.terminate();
        ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());
        ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
        ASSERT_TRUE(component.terminated());
        ASSERT_FALSE(component.is_ready());
    }

    // Terminate from Initialized/Reset
    {
        TestComponent component;
        component.initialize();
        component.terminate();
        ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());
        ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
        ASSERT_TRUE(component.terminated());
    }

    // Terminate from Running
    {
        TestComponent component;
        component.initialize();
        component.execute();
        component.terminate();
        ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());
        ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
    }

    // Terminate is idempotent
    {
        TestComponent component;
        component.initialize();
        component.terminate();
        ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());

        // Second terminate should be no-op
        component.terminate();
        ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());
    }
}

// Test count behavior
TEST(count_behavior) {
    TestComponent component(10);

    // Count is 0 when uninitialized
    ASSERT_EQ(0u, component.count());

    // Count is 0 after initialization
    component.initialize();
    ASSERT_EQ(0u, component.count());

    // Count increments with execute
    component.execute();
    ASSERT_EQ(1u, component.count());
    component.execute();
    ASSERT_EQ(2u, component.count());

    // Count resets to 0 after reset()
    component.reset();
    ASSERT_EQ(0u, component.count());

    // Count increments again after reset
    component.execute();
    ASSERT_EQ(1u, component.count());

    // Count becomes 0 after terminate
    component.terminate();
    ASSERT_EQ(0u, component.count());
}

// Test is_ready() in all states
TEST(is_ready_states) {
    TestComponent component(2);

    // Not ready when uninitialized
    ASSERT_FALSE(component.is_ready());

    // Ready after initialization
    component.initialize();
    ASSERT_TRUE(component.is_ready());

    // Ready while running
    component.execute();
    ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());
    ASSERT_TRUE(component.is_ready());

    // Not ready when done
    component.execute();
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
    ASSERT_FALSE(component.is_ready());

    // Ready again after reset
    component.reset();
    ASSERT_TRUE(component.is_ready());

    // Not ready after terminate
    component.terminate();
    ASSERT_FALSE(component.is_ready());
}

// Test complete lifecycle
TEST(complete_lifecycle) {
    TestComponent component(2);

    // Start uninitialized
    ASSERT_EQ_ENUM(LifecycleState::Uninitialized, component.lifecycle_state());

    // Initialize
    auto init_result = component.initialize();
    ASSERT_FALSE(init_result);
    ASSERT_EQ_ENUM(LifecycleState::Initialized, component.lifecycle_state());
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());

    // Execute to completion
    while (!component.execute()) {
        ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());
    }
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());

    // Reset for another run
    component.reset();
    ASSERT_EQ_ENUM(ExecutionState::Reset, component.execution_state());

    // Execute again
    component.execute();
    ASSERT_EQ_ENUM(ExecutionState::Running, component.execution_state());

    // Terminate
    component.terminate();
    ASSERT_EQ_ENUM(LifecycleState::Terminated, component.lifecycle_state());
    ASSERT_EQ_ENUM(ExecutionState::Done, component.execution_state());
}

int main() {
    std::cout << "Running Component Interface Tests\n";
    std::cout << "==================================\n";

    // All tests run automatically via static initialization

    std::cout << "==================================\n";
    std::cout << "All tests passed!\n";
    return 0;
}