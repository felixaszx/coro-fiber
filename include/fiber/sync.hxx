#ifndef FIBER_SYNC_HXX
#define FIBER_SYNC_HXX

#include "fiber.hxx"

namespace fiber
{
    struct mutex
    {
      protected:
        std::atomic_bool m_ = false;

      public:
        bool //
        try_lock [[nodiscard]] () noexcept;

        coro_ctx //
        lock [[nodiscard]] () noexcept;

        void //
        unlock() noexcept;
    };
}; // namespace fiber

#endif // FIBER_SYNC_HXX
