#ifndef FIBER_THR_CTX_HXX
#define FIBER_THR_CTX_HXX

#include <deque>

#include "fwd.hxx"
#include "coro.hxx"
#include "concurrentqueue.h"

namespace fiber
{
    struct shared_ctx
    {
        friend thr_ctx;

      protected:
        using shared_queue = moodycamel::ConcurrentQueue<coro_handle>;
        using ptok = moodycamel::ProducerToken;
        using ctok = moodycamel::ConsumerToken;

        shared_queue queue_;
        shared_queue low_priority_;
    };

    struct thr_ctx
    {
        friend fiber;

      protected:
        bool allow_stealing_ = true;
        shared_ctx::ptok ptok_;
        shared_ctx::ctok ctok_;
        coro_handle curr_fiber_ = {};
        shared_ctx& shared_ctx_;
        mutable std::deque<coro_handle> background_fibers_ = {};

        inline void //
        schedule(coro_handle h) const;

        inline constexpr bool //
        pre_run(coro_state& state, bool& exec) noexcept;

        inline constexpr void //
        post_run(coro_state& state, bool& reschedule) noexcept;

        inline constexpr bool //
        fetch_foreground(coro_handle& h);

        inline constexpr bool //
        fetch_background(coro_handle& h);

        inline constexpr bool //
        fetch_low_priority(coro_handle& h);

        template <std::uint8_t code>
        inline constexpr bool //
        fetch_with_code(coro_handle& h);

      public:
        using id = std::uintptr_t;

        inline thr_ctx(shared_ctx& shared_ctx) noexcept;

        inline constexpr coro_handle //
        next_fiber() noexcept;

        template <std::uint8_t foreground, std::uint8_t background, std::uint8_t low_priority>
        inline constexpr coro_handle //
        next_fiber_with_order() noexcept;

        inline void //
        run_fiber(coro_handle h) noexcept;

        inline id //
        get_id() const noexcept;
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

inline fiber::thr_ctx::thr_ctx(shared_ctx& shared_ctx) noexcept
    : ptok_(shared_ctx.queue_),
      ctok_(shared_ctx.queue_),
      shared_ctx_(shared_ctx)
{
}

inline void //
fiber::thr_ctx::schedule(coro_handle h) const
{
    auto& coro_state = h.promise();
    if (coro_state.backgroud_)
    {
        background_fibers_.push_back(h);
    }
    else
    {
        if (!shared_ctx_.queue_.enqueue(ptok_, h))
        {
            // shared_ctx::queue_ memory allocation fail
            std::terminate();
        }
    }
}

inline constexpr bool //
fiber::thr_ctx::pre_run(coro_state& state, bool& exec) noexcept
{
    using namespace std::chrono_literals;

    if (!state.lock_.try_lock())
    {
        schedule(curr_fiber_);
        return false;
    }

    switch (state.ctrl_)
    {
        case ctrl_code::wait_for_cond:
        {
            if (state.ctrl_state_.wait_for_cond_.func_())
            {
                state.backgroud_ = state.ctrl_state_.wait_for_cond_.was_backgroud_;
                exec = true;
            }
            else
            {
                state.backgroud_ = true;
            }
            break;
        }
        case ctrl_code::wait_for_join:
        {
            auto& jf = state.ctrl_state_.wait_for_join_.promise();
            if (!jf.lock_.try_lock())
            {
                break;
            }

            exec = state.ctrl_state_.wait_for_join_.done();
            jf.lock_.unlock();
            break;
        }
        case ctrl_code::reschedule_n_get_ctx:
        {
            *state.ctrl_state_.reschedule_n_get_ctx_ = this;
            exec = true;
            break;
        }
        case ctrl_code::sleep_until:
        {
            if (std::chrono::high_resolution_clock::now() >= state.ctrl_state_.sleep_until_)
            {
                exec = true;
                break;
            }
            break;
        }
        case ctrl_code::lock_mutex:
        {
            exec = false;
            break;
        }
        case ctrl_code::reschedule:
        case ctrl_code::reschedule_to_low_priority:
        case ctrl_code::switch_to_background:
        case ctrl_code::bring_to_foreground:
        {
            exec = true;
        }
        case ctrl_code::resume:
        case ctrl_code::noop:
        {
            exec = !curr_fiber_.done();
            break;
        }
    }

    return true;
}

inline constexpr void //
fiber::thr_ctx::post_run(coro_state& state, bool& reschedule) noexcept
{
    switch (state.ctrl_)
    {
        case ctrl_code::wait_for_cond:
        case ctrl_code::bring_to_foreground:
        case ctrl_code::switch_to_background:
        case ctrl_code::reschedule:
        case ctrl_code::reschedule_n_get_ctx:
        case ctrl_code::wait_for_join:
        case ctrl_code::sleep_until:
        case ctrl_code::lock_mutex:
        {
            reschedule = true;
            break;
        }
        case ctrl_code::reschedule_to_low_priority:
        {
            if (!shared_ctx_.low_priority_.enqueue(curr_fiber_))
            {
                // shared_ctx::queue_ memory allocation fail
                std::terminate();
            }
            reschedule = false;
            break;
        }
        case ctrl_code::resume:
        case ctrl_code::noop:
        {
            reschedule = false;
            break;
        }
    }

    state.lock_.unlock();
}

inline constexpr bool //
fiber::thr_ctx::fetch_foreground(coro_handle& h)
{
    return shared_ctx_.queue_.try_dequeue(ctok_, h);
}

inline constexpr bool //
fiber::thr_ctx::fetch_background(coro_handle& h)
{
    if (!background_fibers_.empty())
    {
        h = background_fibers_.front();
        background_fibers_.pop_front();
        return true;
    }

    return false;
}

inline constexpr bool //
fiber::thr_ctx::fetch_low_priority(coro_handle& h)
{
    return shared_ctx_.low_priority_.try_dequeue(h);
}

template <std::uint8_t code>
inline constexpr bool //
fiber::thr_ctx::fetch_with_code(coro_handle& h)
{
    if constexpr (code == 0)
    {
        return fetch_foreground(h);
    }
    else if constexpr (code == 1)
    {
        return fetch_background(h);
    }
    else if constexpr (code == 2)
    {
        return fetch_low_priority(h);
    }

    return false;
}

inline constexpr fiber::coro_handle //
fiber::thr_ctx::next_fiber() noexcept
{
    return next_fiber_with_order<0, 1, 2>();
}

template <std::uint8_t foreground, std::uint8_t background, std::uint8_t low_priority>
inline constexpr fiber::coro_handle //
fiber::thr_ctx::next_fiber_with_order() noexcept
{
    coro_handle next = {};

    if (fetch_with_code<foreground>(next))
    {
        return next;
    }

    if (fetch_with_code<background>(next))
    {
        return next;
    }

    if (fetch_with_code<low_priority>(next))
    {
        return next;
    }

    return {};
}

inline void //
fiber::thr_ctx::run_fiber(coro_handle h) noexcept
{
    curr_fiber_ = h;
    bool exec = false;
    bool reschedule = false;
    auto& fiber_state = h.promise();

    if (!pre_run(fiber_state, exec))
    {
        return;
    }

    if (exec)
    {
        fiber_state.ctrl_ = ctrl_code::resume;
        fiber_state.ctrl_state_ = {};
        fiber_state.curr_ctx_ = this;
        curr_fiber_.resume();

        if (curr_fiber_.done())
        {
            fiber_state.lock_.unlock();
            if (fiber_state.detached_)
            {
                curr_fiber_.destroy();
            }
            return;
        }
    }

    post_run(fiber_state, reschedule);

    if (reschedule)
    {
        schedule(curr_fiber_);
    }
}

inline fiber::thr_ctx::id //
fiber::thr_ctx::get_id() const noexcept
{
    return reinterpret_cast<id>(this);
}

#endif // FIBER_THR_CTX_HXX
