#ifndef FIBER_FIBER_HXX
#define FIBER_FIBER_HXX

#include "coro.hxx"
#include "thr_ctx.hxx"
#include "ctrl.hxx"

namespace fiber
{
    template <typename C, typename... Args>
    concept fiber_coroutine = std::same_as<std::invoke_result_t<C, void>, coro> || //
                              std::same_as<std::invoke_result_t<C, Args...>, coro>;

    template <typename T>
    concept deferencible_to_thr_ctx = requires(T p) //
    {
        { *p } -> std::convertible_to<const thr_ctx&>;
    };

    template <typename T>
    concept thr_ctx_handle = std::convertible_to<T, const thr_ctx&> || //
                             deferencible_to_thr_ctx<T>;

    struct launch
    {
        launch() = delete;

        enum flags : std::int8_t
        {
            foreground_attached = 0,
            background = 1 << 1,
            detached = 1 << 2,
        };
    };

    struct fiber
    {
      protected:
        coro_handle h_ = {};

        inline void //
        launc_now(const thr_ctx& ctx, coro_handle h, bool background, bool detached) noexcept
        {
            h.promise().backgroud_ = background;
            h.promise().detached_ = detached;
            ctx.schedule(h);

            if (!detached)
            {
                h_ = h;
            }
        }

      public:
        using id = uintptr_t;

        fiber(const fiber&) = delete;
        fiber& operator=(const fiber&) = delete;

        inline constexpr operator coro_handle() const noexcept { return h_; }
        inline constexpr fiber(fiber&& other) { *this = std::move(other); }
        inline constexpr fiber& operator=(fiber&& other);
        inline constexpr fiber() noexcept = default;
        inline constexpr ~fiber() noexcept;

        inline constexpr fiber(std::underlying_type_t<launch::flags> l,
                               thr_ctx_handle auto&& ctx,
                               auto&& func,
                               auto&&... args) noexcept
            requires fiber_coroutine<decltype(func), decltype(args)...>
        {
            bool background_fiber = l & launch::background;
            bool detached_fiber = l & launch::detached;

            if constexpr (deferencible_to_thr_ctx<decltype(ctx)>)
            {
                launc_now(*ctx, func(std::forward<decltype(args)>(args)...), background_fiber, detached_fiber);
            }
            else
            {
                launc_now(ctx, func(std::forward<decltype(args)>(args)...), background_fiber, detached_fiber);
            }
        }

        inline id //
        get_id [[nodiscard]] () const noexcept;

        inline constexpr bool //
        joinable [[nodiscard]] () const noexcept;

        inline constexpr ctrl::join_fiber //
        join [[nodiscard]] () noexcept;
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

inline constexpr fiber::fiber& //
fiber::fiber::operator=(fiber&& other)
{
    h_ = other.h_;
    other.h_ = {};
    return *this;
}

inline constexpr fiber::fiber::~fiber() noexcept
{
    if (h_)
    {
        h_.destroy();
    }
}

inline fiber::fiber::id //
fiber::fiber::get_id [[nodiscard]] () const noexcept
{
    return reinterpret_cast<id>(h_.address());
}

inline constexpr bool //
fiber::fiber::joinable [[nodiscard]] () const noexcept
{
    return static_cast<bool>(h_);
}

inline constexpr fiber::ctrl::join_fiber //
fiber::fiber::join [[nodiscard]] () noexcept
{
    return {h_};
}

#endif // FIBER_FIBER_HXX
