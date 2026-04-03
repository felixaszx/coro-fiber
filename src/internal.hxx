#ifndef FIBER_INTERNAL_HXX
#define FIBER_INTERNAL_HXX

#include "std_extension.hxx"
#include "fiber/fiber.hxx"
#include "fiber/ctrl.hxx"
#include "fiber/sync.hxx"

#include <deque>

namespace fiber
{
    enum class ctrl : u32
    {
        new_fiber = 0,
        yield,
        yield_to,
        wait_for_join,
        lock_mutex,
        resume,
        start_stealing,
        stop_stealing,
        noop = max_v,
    };

    union ctrl_data
    {
        struct
        {
            bool success_;
            mutex* mtx_;
        } lock_mutex_;
        coro_handle wait_for_join_;
        coro_handle yield_to_;
    };

    using fiber_queue = std::deque<coro_handle>;

    struct thr_ctx
    {
        bool allow_stealing_ = true;
        std::mutex lock_ = {};
        fiber_queue queue_ = {};
        coro_handle curr_fiber_ = {};
        ctrl ctrl_ = ctrl::noop;
        ctrl_data ctrl_data_ = {};
    };

    using ctx_ref = unique<thr_ctx*[]>;

    struct property::impl
    {
        spinlock lock_ = {};
        ctrl ctrl_ = ctrl::new_fiber;
        ctrl_data ctrl_data_ = {};

        // No lock is required for fields below
        atomic_bool detached_ = false;
    };

#define prop_of(fiber_handle) (*fiber_handle.promise().impl_)

    struct internal
    {
        inline static bool //
        lock_run(coro_handle h, const std::function<void()>& func, bool blocked = false) noexcept
        {
            return lock_run(prop_of(h), func, blocked);
        }

        inline static bool //
        lock_run(property::impl& prop, const std::function<void()>& func, bool blocked = false) noexcept
        {
            auto& lk = prop.lock_;
            if (blocked)
            {
                lk.lock();
            }
            else
            {
                if (!lk.try_lock())
                {
                    return false;
                }
            }

            func();

            lk.unlock();
            return true;
        }
    };
}; // namespace fiber

#endif // FIBER_INTERNAL_HXX
