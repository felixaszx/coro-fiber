/**
 * @file fiber.hxx
 * @author Felixaszx (felixaszx@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-03-29
 * 
 * @copyright APACHEv2 License Copyright (c) 2026
 * 
 */

#ifndef CORO_FIBER_FIBER_HXX
#define CORO_FIBER_FIBER_HXX

#include <deque>
#include <vector>
#include <flat_map>
#include <coroutine>

namespace fbc
{
    struct fiber_ctx;
    struct fiber_promise;
    struct fiber_thr;
    struct fiber;
    struct fiber_coro;

    namespace this_thread
    {
        void //
        schedule_fiber(const fiber& fiber) noexcept;

        bool //
        no_fibers() noexcept;

        void //
        next_fiber() noexcept;

        bool //
        fiber_thr_running() noexcept;
    }; // namespace this_thread

    enum class yield_for : uint8_t
    {
        reschedule = 0,
        join = 1,
        noop = std::numeric_limits<uint8_t>::max(),
    };

    using fiber_handle = std::coroutine_handle<fiber_promise>;
    struct fiber_coro : fiber_handle
    {
      protected:
      public:
        using promise_type = fiber_promise;
    };

    struct fiber_promise
    {
        friend fiber;
        friend void this_thread::next_fiber() noexcept;

      protected:
        bool detached_ = false;
        int return_ = 0;
        std::deque<fiber_handle> notify_queue_ = {};

      public:
        ~fiber_promise() noexcept;

        inline constexpr int get_return [[nodiscard]] () const { return return_; }

        inline constexpr fiber_coro //
        get_return_object() noexcept
        {
            return {fiber_coro::from_promise(*this)};
        }

        inline static constexpr auto //
        initial_suspend() noexcept
        {
            return std::suspend_always();
        }

        inline static constexpr auto //
        final_suspend() noexcept
        {
            return std::suspend_always();
        }

        inline constexpr void //
        return_value(decltype(return_) r) noexcept
        {
            return_ = r;
        }

        inline static constexpr void //
        unhandled_exception() noexcept
        {
        }

        fiber_thr& //
        yield_value(yield_for op) noexcept;
    };

    // per thread
    struct fiber_thr
    {
      public:
        std::deque<fiber_handle> ready_queue_ = {};

        ~fiber_thr() noexcept;

        inline static constexpr bool //
        await_ready [[nodiscard]] () noexcept
        {
            return false;
        }

        void //
        await_suspend(fiber_handle h) noexcept;

        void //
        await_resume() noexcept;
    };

    struct fiber
    {
      protected:
        fiber_coro coro_ = {};

      public:
        inline constexpr fiber(auto&& fiber_func, auto&&... args) noexcept
            requires std::same_as<std::invoke_result_t<std::remove_cvref_t<decltype(fiber_func)>, decltype(args)...>,
                                  fiber_coro>
        {
            coro_ = fiber_func(std::forward<decltype(args)>(args)...);
            this_thread::schedule_fiber(*this);
        }

        inline constexpr fiber(auto&& fiber_func) noexcept
            requires std::same_as<std::invoke_result_t<std::remove_cvref_t<decltype(fiber_func)>>, fiber_coro>
        {
            coro_ = fiber_func();
            this_thread::schedule_fiber(*this);
        }

        inline constexpr fiber() noexcept = default;
        ~fiber() noexcept;

        inline constexpr fiber_coro //
        coro [[nodiscard]] () const noexcept
        {
            return coro_;
        }

        yield_for //
        join [[nodiscard]] () const noexcept;

        void //
        detach() noexcept;
    };

    struct mutex
    {
      protected:
        struct mutex_awaitable
        {
            friend mutex;

          protected:
            bool locked_ = false;
            std::deque<fiber_handle> waiting_queue_ = {};

          public:
            bool //
            await_ready [[nodiscard]] () noexcept;

            void //
            await_suspend(fiber_handle h) noexcept;

            void //
            await_resume() noexcept;
        };

        mutex_awaitable awaitable_ = {};

      public:
        mutex_awaitable& //
        lock [[nodiscard]] () noexcept;

        void //
        unlock() noexcept;
    };
}; // namespace fbc

#endif // CORO_FIBER_FIBER_HXX
