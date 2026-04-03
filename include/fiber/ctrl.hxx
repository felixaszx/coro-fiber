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
        noop [[nodiscard]] () noexcept;

        static fiber::id //
        get_id [[nodiscard]] () noexcept;
    };

    struct this_thread
    {
        friend fiber;
        friend coro_ctx;

      protected:
        static void //
        schedule(coro_handle h, bool reschedule = false) noexcept;

        static void //
        schedule_next(coro_handle h) noexcept;

      public:
        this_thread() = delete;

        static coro_handle
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
