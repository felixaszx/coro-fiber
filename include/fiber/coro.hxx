#ifndef FIBER_CORO_HXX
#define FIBER_CORO_HXX

#include <atomic>
#include <thread>
#include <functional>

#include "fwd.hxx"

namespace fiber
{
    enum class ctrl_code : std::uint8_t
    {
        reschedule,
        reschedule_n_get_ctx,
        reschedule_to_low_priority,
        wait_for_join,
        wait_for_cond,
        switch_to_background,
        bring_to_foreground,
        sleep_until,
        lock_mutex,
        resume,
        noop = std::numeric_limits<std::uint8_t>::max()
    };

    union ctrl_state
    {
        coro_handle wait_for_join_;
        std::chrono::time_point<std::chrono::high_resolution_clock> sleep_until_;
        const thr_ctx** reschedule_n_get_ctx_;

        struct
        {
            bool was_backgroud_ = false;
            std::reference_wrapper<const std::function<bool()>> func_;
        } wait_for_cond_;
    };

    struct coro_state
    {
        friend fiber;
        friend thr_ctx;
        friend ctrl::query<coro_state>;

      public:
        struct state_ref
        {
            friend ctrl::query<coro_state>;

          protected:
            const coro_state* ref_ = nullptr;
            inline constexpr state_ref() = default;

          public:
            inline constexpr bool in_backgroud [[nodiscard]] () const { return ref_->backgroud_; }
            inline constexpr ctrl_code ctrl [[nodiscard]] () const { return ref_->ctrl_; }
            inline constexpr bool detached [[nodiscard]] () const { return ref_->detached_; }
            inline constexpr ctrl_state ctrl_state [[nodiscard]] () const { return ref_->ctrl_state_; }
            inline constexpr const thr_ctx* curr_ctx [[nodiscard]] () const { return ref_->curr_ctx_; }
        };

      protected:
        struct suspend
        {
            inline static constexpr bool await_ready [[nodiscard]] () noexcept { return false; }
            inline static constexpr void await_resume() noexcept {}
            inline static constexpr void await_suspend(coro_handle h) noexcept {}
        };

        struct spinlock
        {
          protected:
            std::atomic_bool m_ = false;

          public:
            inline constexpr bool //
            try_lock [[nodiscard]] () noexcept;

            inline constexpr void //
            lock(std::size_t spin_before_sleep = 16) noexcept;

            inline constexpr void //
            unlock() noexcept;
        };

        bool backgroud_ = false;
        spinlock lock_ = {}; // must set/unset to ensure the memory order is correct
        ctrl_code ctrl_ = ctrl_code::resume;
        bool detached_ = false;
        ctrl_state ctrl_state_ = {};
        const thr_ctx* curr_ctx_ = nullptr;

      public:
        inline static consteval auto initial_suspend() noexcept { return suspend(); }
        inline static consteval auto final_suspend() noexcept { return suspend(); }
        inline static consteval void unhandled_exception() noexcept {}
        inline static consteval void return_void() noexcept {}
        inline constexpr coro get_return_object() noexcept { return {coro::from_promise(*this)}; }

        template <typename C>
        inline constexpr suspend //
        yield_value(C&& ctrl_class);
    };
}; // namespace fiber

//
//
//
//
//
// implementations
//
//
//
//
//

inline constexpr bool //
fiber::coro_state::spinlock::try_lock [[nodiscard]] () noexcept
{
    return !(m_.load(std::memory_order_relaxed) || //
             m_.exchange(true, std::memory_order_acquire));
}

inline constexpr void //
fiber::coro_state::spinlock::lock(std::size_t spin_before_sleep) noexcept
{
    using namespace std::chrono_literals;
    for (std::size_t spins = 0; !try_lock(); spins++)
    {
        if (spins == spin_before_sleep)
        {
            spins = 0;
            std::this_thread::sleep_for(1ns);
        }
    }
}

inline constexpr void //
fiber::coro_state::spinlock::unlock() noexcept
{
    m_.store(false, std::memory_order_release);
}

template <typename C>
inline constexpr fiber::coro_state::suspend //
fiber::coro_state::yield_value(C&& ctrl_class)
{
    ctrl_ = C::ctrl_;

    if constexpr (C::has_ctrl_call_)
    {
        std::forward<C>(ctrl_class)(*this);
    }

    if constexpr (C::has_ctrl_state_)
    {
        ctrl_state_ = std::forward<C>(ctrl_class).ctrl_state_;
    }

    // control specific functions

    if constexpr (C::ctrl_ == ctrl_code::wait_for_cond)
    {
        ctrl_state_.wait_for_cond_.was_backgroud_ = backgroud_;
    }
    else if constexpr (C::ctrl_ == ctrl_code::bring_to_foreground)
    {
        backgroud_ = false;
    }
    else if constexpr (C::ctrl_ == ctrl_code::switch_to_background)
    {
        backgroud_ = true;
    }

    return {};
}

#endif // FIBER_CORO_HXX
