#ifndef FIBER_FIBER_HXX
#define FIBER_FIBER_HXX

#include <atomic>
#include <memory>
#include <coroutine>
#include <type_traits>
#include <functional>
#include <cstdint>

#include "fwd.hxx"

namespace fiber
{
    struct property
    {
        friend fiber;
        friend coro_ctx;
        friend this_fiber;
        friend this_thread;
        friend internal;

      public:
        struct impl;

      protected:
        std::unique_ptr<impl> impl_ = {};

      public:
        property() noexcept;
        ~property() noexcept;
        inline static constexpr auto initial_suspend() noexcept { return std::suspend_always(); }
        inline static constexpr auto final_suspend() noexcept { return std::suspend_always(); }
        inline static constexpr void return_void() noexcept {}
        inline static constexpr void unhandled_exception() noexcept {}
        inline static constexpr auto&& yield_value(coro_ctx&& ctx) noexcept { return ctx; }
        inline constexpr coro get_return_object() noexcept { return {coro::from_promise(*this)}; }
    };

    struct coro_ctx
    {
        friend fiber;
        friend this_fiber;
        friend this_thread;
        friend mutex;

      protected:
        inline constexpr coro_ctx() = default;

        struct awaitable
        {
            static bool //
            await_ready [[nodiscard]] () noexcept;

            static void //
            await_suspend(coro_handle h) noexcept;

            inline static constexpr void //
            await_resume() noexcept
            {
            }
        };

      public:
        inline constexpr awaitable operator co_await [[nodiscard]] () const noexcept { return {}; }
    };

    struct fiber
    {
        friend internal;
        friend coro_ctx;
        friend this_thread;

      protected:
        coro_handle h_ = {};

      public:
        using id = uintptr_t;
        inline static const id invalid_id = reinterpret_cast<id>(nullptr);

        fiber() = default;

        fiber(const fiber&) = delete;
        fiber& operator=(const fiber&) = delete;

        ~fiber() noexcept;
        inline constexpr operator coro_handle() const noexcept { return h_; }
        inline constexpr fiber(fiber&& other) { *this = std::move(other); }
        inline constexpr fiber& operator=(fiber&& other)
        {
            h_ = other.h_;
            other.h_ = {};
            return *this;
        }

        inline constexpr fiber(auto&& func, auto&&... args) noexcept
            requires std::same_as<std::invoke_result_t<decltype(func), decltype(args)...>, coro>
        {
            launch(func(std::forward<decltype(args)>(args)...));
        }

        inline constexpr fiber(auto&& func) noexcept
            requires std::same_as<std::invoke_result_t<decltype(func)>, coro>
        {
            launch(func());
        }

        void //
        launch(coro_handle h) noexcept;

        id //
        get_id [[nodiscard]] () const noexcept;

        void //
        detach() noexcept;

        coro_ctx //
        join [[nodiscard]] () noexcept;

        bool //
        joinable [[nodiscard]] () const noexcept;
    };
}; // namespace fiber

#endif // FIBER_FIBER_HXX
