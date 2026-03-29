/**
 * @file fiber.hxx
 * @author Felixaszx (felixaszx@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-03-29
 * 
 * @copyright APACHEv3 License Copyright (c) 2026
 * 
 */

#include "fiber.hxx"
#include <algorithm>

namespace
{
    static thread_local fbc::fiber_thr local_thr = {};
    static thread_local fbc::yield_for ctrl_code = fbc::yield_for::noop;
    static thread_local std::deque<fbc::fiber_handle>* join_notify = nullptr;
}; // namespace

fbc::fiber_thr::~fiber_thr() noexcept
{
}

void //
fbc::fiber_thr::await_suspend(fiber_handle h) noexcept
{
    switch (ctrl_code)
    {
        case yield_for::reschedule:
        {
            ready_queue_.push_back(h);
            break;
        }
        case yield_for::join:
        {
            if (join_notify != nullptr)
            {
                join_notify->push_back(h);
                join_notify = nullptr;
            }
            else
            {
                ready_queue_.push_front(h);
            }
            break;
        }
        case yield_for::noop:
        {
            ready_queue_.push_front(h);
            break;
        }
    }
    ctrl_code = yield_for::noop;
}

void //
fbc::fiber_thr::await_resume() noexcept
{
}

fbc::fiber_promise::~fiber_promise() noexcept
{
}

fbc::fiber_thr& //
fbc::fiber_promise::yield_value(yield_for op) noexcept
{
    ::ctrl_code = op;
    return local_thr;
}

void //
fbc::this_thread::schedule_fiber(const fiber& fiber) noexcept
{
    local_thr.ready_queue_.push_back(fiber.coro());
}

bool //
fbc::this_thread::no_fibers() noexcept
{
    return local_thr.ready_queue_.empty();
}

void //
fbc::this_thread::next_fiber() noexcept
{
    auto h = local_thr.ready_queue_.front();
    local_thr.ready_queue_.pop_front();

    if (!h.done())
    {
        h.resume();
    }

    if (h.done())
    {
        auto& notify_queue = h.promise().notify_queue_;
        local_thr.ready_queue_.insert_range(local_thr.ready_queue_.begin(), notify_queue);
        if (h.promise().detached_)
        {
            h.destroy();
        }
    }
}

bool //
fbc::this_thread::fiber_thr_running() noexcept
{
    if (!no_fibers())
    {
        next_fiber();
        return true;
    }
    else
    {
        return false;
    }
}

fbc::fiber::~fiber() noexcept
{
    if (!coro_)
    {
        return;
    }

    if (!coro_.done())
    {
        detach();
        return;
    }

    if (!coro_.promise().detached_)
    {
        coro_.destroy();
    }
}

fbc::yield_for //
fbc::fiber::join [[nodiscard]] () const noexcept
{
    if (!coro_.done() && !coro_.promise().detached_)
    {
        join_notify = &this->coro_.promise().notify_queue_;
        return yield_for::join;
    }
    return yield_for::noop;
}

void //
fbc::fiber::detach() noexcept
{
    if (!coro_.done())
    {
        coro_.promise().detached_ = true;
    }
}

bool //
fbc::mutex::mutex_awaitable::await_ready [[nodiscard]] () noexcept
{
    if (!locked_)
    {
        locked_ = true;
        return true;
    }
    return false;
}

void //
fbc::mutex::mutex_awaitable::await_suspend(fiber_handle h) noexcept
{
    waiting_queue_.push_back(h);
}

void //
fbc::mutex::mutex_awaitable::await_resume() noexcept
{
}

fbc::mutex::mutex_awaitable& //
fbc::mutex::lock [[nodiscard]] () noexcept
{
    return awaitable_;
}

void //
fbc::mutex::unlock() noexcept
{
    auto& queue = awaitable_.waiting_queue_;
    if (queue.empty())
    {
        awaitable_.locked_ = false;
    }
    else
    {
        local_thr.ready_queue_.push_back(queue.front());
        queue.pop_front();
    }
}
