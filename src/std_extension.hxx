/**
 * @file std_extention.hxx
 * @author Felixaszx (felixaszx@outlook.com)
 * @brief Core defines and help tempaltes
 * @version 0.1
 * @date 2025-07-12
 *
 * @copyright MIT License Copyright (c) 2025
 *
 */

#ifndef STD_EXTENTION_HXX
#define STD_EXTENTION_HXX

#include <print>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
#include <type_traits>
#include <source_location>
#include <pthread.h>

#define STD_EXT_HPP_NAMESPACE         std_ext
#define STD_EXT_HPP_NAMESPACE_CAPITAL STD_EXT

namespace STD_EXT_HPP_NAMESPACE::i_ // internal namespace
{
    template <typename... F>
    struct overload_call_t : F...
    {
        using F::operator()...;
    };

    template <typename L>
    struct limit_v_t
    {
        template <typename M>
        inline static const M as = L::template as<M>;
    };

    template <typename M, typename L>
    inline constexpr bool //
    operator<(const M& x, const limit_v_t<L>& y)
    {
        return x < limit_v_t<L>::template as<M>;
    }

    template <typename M, typename L>
    inline constexpr bool //
    operator<=(const M& x, const limit_v_t<L>& y)
    {
        return x <= limit_v_t<L>::template as<M>;
    }

    template <typename M, typename L>
    inline constexpr bool //
    operator>(const M& x, const limit_v_t<L>& y)
    {
        return x > limit_v_t<L>::template as<M>;
    }

    template <typename M, typename L>
    inline constexpr bool //
    operator>=(const M& x, const limit_v_t<L>& y)
    {
        return x >= limit_v_t<L>::template as<M>;
    }

    template <typename M, typename L>
    inline constexpr bool //
    operator==(const M& x, const limit_v_t<L>& y)
    {
        return x == limit_v_t<L>::template as<M>;
    }

    template <bool S = false>
    struct max_v_t : public limit_v_t<max_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::max();
            }
            else
            {
                return std::numeric_limits<M>::max();
            }
        }

        inline consteval const max_v_t<!S> //
        operator-() const
        {
            return max_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::max();
    };

    template <bool S = false>
    struct min_v_t : public limit_v_t<min_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::min();
            }
            else
            {
                return std::numeric_limits<M>::min();
            }
        }

        inline consteval const min_v_t<!S> //
        operator-() const
        {
            return min_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::min();
    };

    template <bool S = false>
    struct inf_v_t : public limit_v_t<inf_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::infinity();
            }
            else
            {
                return std::numeric_limits<M>::infinity();
            }
        }

        inline consteval const inf_v_t<!S> //
        operator-() const
        {
            return inf_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::infinity();
    };

    template <bool S = false>
    struct epsilon_v_t : public limit_v_t<epsilon_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::epsilon();
            }
            else
            {
                return std::numeric_limits<M>::epsilon();
            }
        }

        inline consteval const epsilon_v_t<!S> //
        operator-() const
        {
            return epsilon_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::epsilon();
    };

    template <bool S = false>
    struct lowest_v_t : public limit_v_t<lowest_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::lowest();
            }
            else
            {
                return std::numeric_limits<M>::lowest();
            }
        }

        inline consteval const lowest_v_t<!S> //
        operator-() const
        {
            return lowest_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::lowest();
    };

    struct quiet_NaN_v_t : public limit_v_t<quiet_NaN_v_t>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            return std::numeric_limits<M>::quiet_NaN();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::quiet_NaN();
    };

    struct sig_NaN_v_t : public limit_v_t<sig_NaN_v_t>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            return std::numeric_limits<M>::signaling_NaN();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::signaling_NaN();
    };

    template <bool S = false>
    struct round_error_v_t : public limit_v_t<round_error_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::round_error();
            }
            else
            {
                return std::numeric_limits<M>::round_error();
            }
        }

        inline consteval const round_error_v_t<!S> //
        operator-() const
        {
            return round_error_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::round_error();
    };

    template <bool S = false>
    struct denorm_min_v_t : public limit_v_t<denorm_min_v_t<S>>
    {
        template <typename M>
        inline consteval //
        operator M() const
        {
            if constexpr (S)
            {
                return -std::numeric_limits<M>::denorm_min();
            }
            else
            {
                return std::numeric_limits<M>::denorm_min();
            }
        }

        inline consteval const denorm_min_v_t<!S> //
        operator-() const
        {
            return denorm_min_v_t<!S>();
        }

        template <typename M>
        inline static const M as = std::numeric_limits<M>::denorm_min();
    };
}; // namespace STD_EXT_HPP_NAMESPACE::i_

namespace STD_EXT_HPP_NAMESPACE_CAPITAL
{
    using i8 = std::int8_t;
    using u8 = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;
    using imax = std::intmax_t;
    using umax = std::uintmax_t;
    using usz = std::size_t;
    using ssz = std::ptrdiff_t;
    using ptrdiff = std::ptrdiff_t;
    using uptr = std::uintptr_t;
    using iptr = std::intptr_t;

    using uchar = unsigned char;
    using char8 = char8_t;
    using char16 = char16_t;
    using char32 = char32_t;
    using wchar = wchar_t;

    using f32 = float;
    using f64 = double;
    using flong = long double;

    inline static const std_ext::i_::quiet_NaN_v_t quiet_NaN = {};
    inline static const std_ext::i_::sig_NaN_v_t sig_NaN = {};
    inline static const std_ext::i_::quiet_NaN_v_t nan_v = {};

    inline static const std_ext::i_::min_v_t min_v = {};
    inline static const std_ext::i_::lowest_v_t lowest_v = {};
    inline static const std_ext::i_::max_v_t max_v = {};
    inline static const std_ext::i_::epsilon_v_t epsilon_v = {};
    inline static const std_ext::i_::round_error_v_t round_err_v = {};
    inline static const std_ext::i_::inf_v_t inf_v = {};
    inline static const std_ext::i_::denorm_min_v_t denorm_min_v = {};

    // atomic variants
    using atomic_bool = std::atomic<bool>;
    using atomic_i8 = std::atomic<i8>;
    using atomic_u8 = std::atomic<u8>;
    using atomic_i16 = std::atomic<i16>;
    using atomic_u16 = std::atomic<u16>;
    using atomic_i32 = std::atomic<i32>;
    using atomic_u32 = std::atomic<u32>;
    using atomic_i64 = std::atomic<i64>;
    using atomic_u64 = std::atomic<u64>;
    using atomic_imax = std::atomic<imax>;
    using atomic_umax = std::atomic<umax>;
    using atomic_usz = std::atomic<usz>;
    using atomic_ssz = std::atomic<ssz>;
    using atomic_ptrdiff = std::atomic<ptrdiff>;
    using atomic_uptr = std::atomic<uptr>;
    using atomic_iptr = std::atomic<iptr>;

    using atomic_char = std::atomic<char>;
    using atomic_uchar = std::atomic<uchar>;
    using atomic_char8 = std::atomic<char8>;
    using atomic_char16 = std::atomic<char16>;
    using atomic_char32 = std::atomic<char32>;
    using atomic_wchar = std::atomic<wchar>;

    using atomic_f32 = std::atomic<f32>;
    using atomic_f64 = std::atomic<f64>;
    using atomic_flag = std::atomic_flag;

    template <typename T>
    using atomic = std::atomic<T>;

    template <typename Idx = std::size_t, auto C = std::numeric_limits<std::size_t>::max(), typename R = void>
        requires std::unsigned_integral<Idx>
    struct ts_idx
    {
      public:
        inline static const ts_idx null = static_cast<Idx>(-1);
        inline static const ts_idx zero = static_cast<Idx>(0);
        using idx_t = Idx;

      protected:
        idx_t idx_ = static_cast<idx_t>(-1);

      public:
        inline constexpr operator bool() const noexcept { return valid(); }
        inline constexpr bool operator==(const ts_idx& x) const noexcept { return idx_ == x.idx_; }
        inline constexpr bool operator<=(const ts_idx& x) const noexcept { return idx_ <= x.idx_; }
        inline constexpr bool operator>=(const ts_idx& x) const noexcept { return idx_ >= x.idx_; }
        inline constexpr bool operator<(const ts_idx& x) const noexcept { return idx_ < x.idx_; }
        inline constexpr bool operator>(const ts_idx& x) const noexcept { return idx_ > x.idx_; }
        inline constexpr ts_idx(idx_t i) { idx_ = i; }
        inline static consteval decltype(C) idx_class() noexcept { return C; };
        inline const ts_idx off_by(long long off) const noexcept { return *this ? ts_idx(idx_ + off) : null; };
        inline const Idx number() const noexcept { return idx_; };
        inline const bool valid [[nodiscard]] () const noexcept { return null.idx_ != idx_; };

        template <std::integral I>
        inline constexpr //
        operator I() const noexcept
        {
            return idx_;
        }

        template <std::integral I>
        inline constexpr bool //
        operator==(const I& x) const noexcept
        {
            return idx_ == x;
        }

        template <std::integral I>
        inline constexpr bool //
        operator<=(const I& x) const noexcept
        {
            return idx_ <= x;
        }

        template <std::integral I>
        inline constexpr bool //
        operator>=(const I& x) const noexcept
        {
            return idx_ >= x;
        }

        template <std::integral I>
        inline constexpr bool //
        operator<(const I& x) const noexcept
        {
            return idx_ < x;
        }

        template <std::integral I>
        inline constexpr bool //
        operator>(const I& x) const noexcept
        {
            return idx_ > x;
        }

        ts_idx() = default;
    };

    enum struct cmp_result : signed char
    {
        less = -1,
        equivalent = 0,
        greater = 1
    };

    template <typename T>
        requires(std::same_as<T, std::strong_ordering> || //
                 std::same_as<T, std::weak_ordering> ||   //
                 std::same_as<T, std::partial_ordering>)
    constexpr cmp_result //
    cmp_to_integral(T x)
    {
        return std::bit_cast<cmp_result>(x);
    }

    static_assert(cmp_to_integral(std::strong_ordering::less) == cmp_result::less);
    static_assert(cmp_to_integral(std::strong_ordering::equivalent) == cmp_result::equivalent);
    static_assert(cmp_to_integral(std::strong_ordering::greater) == cmp_result::greater);
    static_assert(cmp_to_integral(std::weak_ordering::less) == cmp_result::less);
    static_assert(cmp_to_integral(std::weak_ordering::equivalent) == cmp_result::equivalent);
    static_assert(cmp_to_integral(std::weak_ordering::greater) == cmp_result::greater);
    static_assert(cmp_to_integral(std::partial_ordering::less) == cmp_result::less);
    static_assert(cmp_to_integral(std::partial_ordering::equivalent) == cmp_result::equivalent);
    static_assert(cmp_to_integral(std::partial_ordering::greater) == cmp_result::greater);
}; // namespace STD_EXT_HPP_NAMESPACE_CAPITAL

namespace STD_EXT_HPP_NAMESPACE
{
    using namespace STD_EXT_HPP_NAMESPACE_CAPITAL;

    inline const std::size_t DEFAULT_ALITNMENT = 16;

    template <typename T>
    inline constexpr auto //
    aligned [[nodiscard]] (const T& size, std::size_t alignment = DEFAULT_ALITNMENT) noexcept
        requires std::is_arithmetic_v<T>
    {
        return alignment * ((size - 1) / alignment) + alignment;
    }

    inline constexpr auto //
    ceili [[nodiscard]] (const std::integral auto& dividend, const std::integral auto& divisor) noexcept
    {
        return dividend == 0 ? 0 : (dividend - 1) / divisor + 1;
    }

    template <typename T>
    inline constexpr auto //
    align(T& size, std::size_t alignment = DEFAULT_ALITNMENT) noexcept
        requires std::is_arithmetic_v<T>
    {
        size = aligned(size, alignment);
        return size;
    }

    struct c_delete
    {
        inline constexpr void //
        operator()(void* p) noexcept
        {
        }
    };

    template <typename Tp, typename Dp = std::default_delete<Tp>>
    using unique = std::unique_ptr<Tp, Dp>;

    template <typename Tp>
    using shared = std::shared_ptr<Tp>;

    template <typename Tp>
    using weak = std::weak_ptr<Tp>;

    template <typename Tp, typename Dp = std::default_delete<Tp>>
    using atomic_unique = std::atomic<unique<Tp, Dp>>;

    template <typename Tp>
    using atomic_shared = std::atomic<shared<Tp>>;

    template <typename Tp>
    using atomic_weak = std::atomic<weak<Tp>>;

    // This outperform libc++'s std::mutex (~2M /s) on Windows 10/11, max at 16 thread Ryzen 7700X, 64M-718M /s
    struct spinlock
    {
      protected:
        atomic_bool m_ = false;

      public:
        inline constexpr bool //
        try_lock [[nodiscard]] () noexcept
        {
            return !(m_.load(std::memory_order_relaxed) || //
                     m_.exchange(true, std::memory_order_acquire));
        }

        inline constexpr void //
        lock(std::size_t spin_before_sleep = 16) noexcept
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
        unlock() noexcept
        {
            m_.store(false, std::memory_order_release);
        }
    };

#define warnln(...) std::cout << std::format(__VA_ARGS__) << '\n'
#define errln(...)  std::cerr << std::format(__VA_ARGS__) << '\n'
#if defined(NDEBUG)
    #define logln(...)
#else
    #define logln(...) std::clog << std::format(__VA_ARGS__) << '\n'
#endif

    template <typename... F>
    inline constexpr i_::overload_call_t<F...> //
    overload(F&&... funcs)
    {
        return i_::overload_call_t{std::forward<F>(funcs)...};
    }
}; // namespace STD_EXT_HPP_NAMESPACE

namespace STD_EXT_HPP_NAMESPACE::literals
{
    using namespace std::chrono_literals;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    inline consteval std::size_t //
    operator""_b(unsigned long long size)
    {
        return size;
    }

    inline consteval std::size_t //
    operator""_kb(unsigned long long size)
    {
        return 1024_b * size;
    }

    inline consteval std::size_t //
    operator""_mb(unsigned long long size)
    {
        return 1024_kb * size;
    }

    inline consteval std::size_t //
    operator""_gb(unsigned long long size)
    {
        return 1024_mb * size;
    }

    inline consteval std::size_t //
    operator""_kb(long double sizef)
    {
        return 1024_b * sizef;
    }

    inline consteval std::size_t //
    operator""_mb(long double sizef)
    {
        return 1024_kb * sizef;
    }

    inline consteval std::size_t //
    operator""_gb(long double sizef)
    {
        return 1024_mb * sizef;
    }

    inline consteval std::int8_t //
    operator""_i8(unsigned long long i)
    {
        return static_cast<std::int8_t>(i);
    }

    inline consteval std::uint8_t //
    operator""_u8(unsigned long long i)
    {
        return static_cast<std::uint8_t>(i);
    }

    inline consteval std::int16_t //
    operator""_i16(unsigned long long i)
    {
        return static_cast<std::int16_t>(i);
    }

    inline consteval std::uint16_t //
    operator""_u16(unsigned long long i)
    {
        return static_cast<std::uint16_t>(i);
    }

    inline consteval std::int32_t //
    operator""_i32(unsigned long long i)
    {
        return static_cast<std::int32_t>(i);
    }

    inline consteval std::uint32_t //
    operator""_u32(unsigned long long i)
    {
        return static_cast<std::uint32_t>(i);
    }

    inline consteval std::int64_t //
    operator""_i64(unsigned long long i)
    {
        return static_cast<std::int64_t>(i);
    }

    inline consteval std::uint64_t //
    operator""_u64(unsigned long long i)
    {
        return static_cast<std::uint64_t>(i);
    }

    inline consteval std::intmax_t //
    operator""_imax(unsigned long long i)
    {
        return static_cast<std::intmax_t>(i);
    }

    inline consteval std::uintmax_t //
    operator""_umax(unsigned long long i)
    {
        return static_cast<std::uintmax_t>(i);
    }

    inline consteval std::size_t //
    operator""_usz(unsigned long long i)
    {
        return static_cast<std::size_t>(i);
    }

    inline consteval std::ptrdiff_t //
    operator""_ssz(unsigned long long i)
    {
        return static_cast<std::ptrdiff_t>(i);
    }

    inline consteval std::ptrdiff_t //
    operator""_ptrdiff(unsigned long long i)
    {
        return static_cast<std::ptrdiff_t>(i);
    }

    inline consteval std::uintptr_t //
    operator""_uptr(unsigned long long i)
    {
        return static_cast<std::uintptr_t>(i);
    }

    inline consteval std::intptr_t //
    operator""_iptr(unsigned long long i)
    {
        return static_cast<std::intptr_t>(i);
    }

    inline consteval float //
    operator""_f32(long double f)
    {
        return static_cast<f32>(f);
    }

    inline consteval float //
    operator""_f64(long double f)
    {
        return static_cast<f64>(f);
    }

    inline consteval float //
    operator""_flong(long double f)
    {
        return static_cast<flong>(f);
    }

    inline consteval float //
    operator""_f32(unsigned long long f)
    {
        return static_cast<f32>(f);
    }

    inline consteval float //
    operator""_f64(unsigned long long f)
    {
        return static_cast<f64>(f);
    }

    inline consteval float //
    operator""_flong(unsigned long long f)
    {
        return static_cast<flong>(f);
    }

}; // namespace STD_EXT_HPP_NAMESPACE::literals

#if !defined(STD_EXT_NO_NAMESPACE)
using namespace STD_EXT_HPP_NAMESPACE_CAPITAL;
using namespace STD_EXT_HPP_NAMESPACE;
using namespace STD_EXT_HPP_NAMESPACE::literals;

namespace std
{
    namespace fs = std::filesystem;
};

    #define castc const_cast
    #define castd dynamic_cast
    #define castr reinterpret_cast
    #define casts static_cast

    #define lengthof(c_arr) (sizeof(c_arr) / sizeof(c_arr[0]))
    #define widthof(x)      (sizeof(x) * CHAR_BIT)

    #define sizeof2(c_arr)                    (c_arr.size() * sizeof(decltype(c_arr)::value_type))
    #define swap_clear(container)             (decltype(container)().swap(container))
    #define stl_expand(container, x)          (container.resize(container.size() + (x)))
    #define stl_expand_capacity(container, x) (container.reserve(container.size() + (x)))

    #define panic std::terminate
    #define panic_if(condition, reasons)                                                 \
        if (condition)                                                                   \
        {                                                                                \
            std::cerr << std::format("CRASHED ({} == true): {}\n", #condition, reasons); \
            panic();                                                                     \
        }

#endif

#endif // STD_EXTENTION_HXX
