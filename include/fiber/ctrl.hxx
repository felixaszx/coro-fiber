#ifndef FIBER_CTRL_HXX
#define FIBER_CTRL_HXX

#include "fiber.hxx"

namespace fiber
{
    struct this_fiber
    {
        this_fiber() = delete;

        static coro_ctx //
        yield [[nodiscard]] () noexcept;

        static coro_ctx //
        yield_to [[nodiscard]] (coro_handle h) noexcept;

        static coro_ctx //
        wait_for [[nodiscard]] (const std::function<bool()>& cond) noexcept;

        static coro_ctx //
        noop [[nodiscard]] () noexcept;

        static fiber::id //
        get_id [[nodiscard]] () noexcept;

        static coro_ctx //
        sleep_until [[nodiscard]] (const std::chrono::time_point<std::chrono::high_resolution_clock>& tp) noexcept;

        inline static constexpr coro_ctx //
        sleep_for [[nodiscard]] (const std::chrono::nanoseconds& duration) noexcept
        {
            return sleep_until(std::chrono::high_resolution_clock::now() + duration);
        }
    };

    struct this_thread
    {
        friend fiber;
        friend coro_ctx;

      public:
        this_thread() = delete;

        static coro_handle //
        pick_next_fiber() noexcept;

        static void //
        run_fiber(coro_handle h) noexcept;

        static bool //
        init_scheduler(bool allow_stealing = true) noexcept;

        static coro_ctx //
        start_stealing [[nodiscard]] () noexcept;

        static coro_ctx //
        stop_stealing [[nodiscard]] () noexcept;
    };
}; // namespace fiber

#endif // FIBER_CTRL_HXX
