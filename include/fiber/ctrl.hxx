#ifndef FIBER_CTRL_HXX
#define FIBER_CTRL_HXX

#include "coro.hxx"

namespace fiber::ctrl
{
    template <>
    struct reschedule<void>
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::reschedule;
        inline static const bool has_ctrl_state_ = false;
        inline static const bool has_ctrl_call_ = false;
    };

    template <>
    struct reschedule<thr_ctx>
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::reschedule_n_get_ctx;
        inline static const bool has_ctrl_state_ = true;
        inline static const bool has_ctrl_call_ = false;

        ctrl_state ctrl_state_ = {};

      public:
        inline constexpr reschedule(const thr_ctx*& ref_ctx) { ctrl_state_.reschedule_n_get_ctx_ = &ref_ctx; }
    };

    template <>
    struct reschedule<low_priority>
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::reschedule_to_low_priority;
        inline static const bool has_ctrl_state_ = false;
        inline static const bool has_ctrl_call_ = false;
    };

    struct wait_for
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::wait_for_cond;
        inline static const bool has_ctrl_state_ = true;
        inline static const bool has_ctrl_call_ = false;

        ctrl_state ctrl_state_ = {};

      public:
        inline constexpr wait_for(const std::function<bool()>& call)
        {
            ctrl_state_.wait_for_cond_.func_ = std::ref(call);
        }
    };

    struct join_fiber
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::wait_for_join;
        inline static const bool has_ctrl_state_ = true;
        inline static const bool has_ctrl_call_ = false;

        ctrl_state ctrl_state_ = {};

      public:
        inline constexpr join_fiber(coro_handle jh) { ctrl_state_.wait_for_join_ = jh; }
    };

    struct sleep_until
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::sleep_until;
        inline static const bool has_ctrl_state_ = true;
        inline static const bool has_ctrl_call_ = false;

        ctrl_state ctrl_state_ = {};

      public:
        inline constexpr sleep_until(std::chrono::time_point<std::chrono::high_resolution_clock> tp)
        {
            ctrl_state_.sleep_until_ = tp;
        }
    };

    struct sleep_for
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::sleep_until;
        inline static const bool has_ctrl_state_ = true;
        inline static const bool has_ctrl_call_ = false;

        ctrl_state ctrl_state_ = {};

      public:
        inline constexpr sleep_for(auto&& duration)
        {
            ctrl_state_.sleep_until_ = std::chrono::high_resolution_clock::now() //
                                       + std::forward<decltype(duration)>(duration);
        }
    };

    struct switch_to_background
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::switch_to_background;
        inline static const bool has_ctrl_state_ = false;
        inline static const bool has_ctrl_call_ = false;
    };

    struct bring_to_foreground
    {
        friend coro_state;

      protected:
        inline static const ctrl_code ctrl_ = ctrl_code::bring_to_foreground;
        inline static const bool has_ctrl_state_ = false;
        inline static const bool has_ctrl_call_ = false;
    };

    // non-context switching controls and queries, co_await these
    template <>
    struct query<coro_state>
    {
      protected:
        coro_state::state_ref s_;

      public:
        inline static consteval bool await_ready [[nodiscard]] () noexcept { return false; }
        inline constexpr coro_state::state_ref await_resume [[nodiscard]] () noexcept { return s_; }

        inline bool //
        await_suspend(coro_handle h) noexcept
        {
            s_.ref_ = &h.promise();
            return false;
        }
    };

    template <>
    struct set<switch_to_background>
    {
      protected:
        const thr_ctx* t_ = nullptr;

      public:
        inline static consteval bool await_ready [[nodiscard]] () noexcept { return false; }
        inline static consteval void await_resume() noexcept {}

        inline bool //
        await_suspend(coro_handle h) noexcept
        {
            h.promise().yield_value(switch_to_background());
            return false;
        }
    };

    template <>
    struct set<bring_to_foreground>
    {
      protected:
        const thr_ctx* t_ = nullptr;

      public:
        inline static consteval bool await_ready [[nodiscard]] () noexcept { return false; }
        inline static consteval void await_resume() noexcept {}

        inline bool //
        await_suspend(coro_handle h) noexcept
        {
            h.promise().yield_value(bring_to_foreground());
            return false;
        }
    };
}; // namespace fiber::ctrl
#endif // FIBER_CTRL_HXX
