#ifndef FIBER_WSQ_HXX
#define FIBER_WSQ_HXX

#include <atomic>
#include <vector>
#include <optional>
#include <cassert>
#include <new>

/**
@class: WorkStealingQueue

@tparam T data type

@brief Lock-free unbounded single-producer multiple-consumer queue.

This class implements the work stealing queue described in the paper,
"Correct and Efficient Work-Stealing for Weak Memory Models,"
available at https://www.di.ens.fr/~zappa/readings/ppopp13.pdf.

Only the queue owner can perform pop and push operations,
while others can steal data from the queue.
*/
template <typename T>
struct work_stealing_queue
{
  protected:
    struct block
    {
      protected:
        ssize_t c_;
        ssize_t m_;
        std::atomic<T>* data_;

      public:
        inline constexpr ~block() = default;
        inline constexpr block(ssize_t c)
            : c_(c),
              m_(c - 1),
              data_(new std::atomic<T>[static_cast<std::size_t>(c)])
        {
        }

        inline constexpr ssize_t //
        capacity [[nodiscard]] () const noexcept
        {
            return c_;
        }

        template <typename O>
        void set(ssize_t i, O&& v) noexcept
        {
            data_[i & m_].store(std::forward<O>(v), std::memory_order_relaxed);
        }

        inline constexpr T //
        load [[nodiscard]] (ssize_t i) const noexcept
        {
            return data_[i & m_].load(std::memory_order_relaxed);
        }

        inline constexpr block* //
        expand [[nodiscard]] (ssize_t front, ssize_t back) const
        {
            block* ptr = new block(2 * c_);
            for (ssize_t i = front; i != back; ++i)
            {
                ptr->set(i, load(i));
            }
            return ptr;
        }
    };

    // avoids false sharing between front_ and back_
#ifdef __cpp_lib_hardware_interference_size
    alignas(std::hardware_destructive_interference_size) std::atomic<ssize_t> front_;
    alignas(std::hardware_destructive_interference_size) std::atomic<ssize_t> back_;
#else
    alignas(64) std::atomic<ssize_t> front_;
    alignas(64) std::atomic<ssize_t> back_;
#endif
    std::atomic<block*> block_;
    std::vector<block*> garbage_;

  public:
    /**
    @brief constructs the queue with a given capacity

    @param capacity the capacity of the queue (must be power of 2)
    */
    inline constexpr work_stealing_queue(ssize_t c = 1024)
    {
        assert(c != 0 && (!(c & (c - 1))));
        front_.store(0, std::memory_order_relaxed);
        back_.store(0, std::memory_order_relaxed);
        block_.store(new block(c), std::memory_order_relaxed);
        garbage_.reserve(32);
    }

    /**
    @brief destructs the queue
    */
    inline constexpr ~work_stealing_queue()
    {
        for (auto blk : garbage_)
        {
            delete blk;
        }
        delete block_.load();
    }

    /**
    @brief queries if the queue is empty at the time of this call
    */
    inline constexpr bool //
    empty [[nodiscard]] () const noexcept
    {
        ssize_t b = back_.load(std::memory_order_relaxed);
        ssize_t f = front_.load(std::memory_order_relaxed);
        return b <= f;
    }

    /**
    @brief queries the number of items at the time of this call
    */
    inline constexpr std::size_t //
    size [[nodiscard]] () const noexcept
    {
        ssize_t b = back_.load(std::memory_order_relaxed);
        ssize_t f = front_.load(std::memory_order_relaxed);
        return static_cast<size_t>(b >= f ? b - f : 0);
    }

    /**
    @brief queries the capacity of the queue
    */
    inline constexpr ssize_t //
    capacity [[nodiscard]] () const noexcept
    {
        return block_.load(std::memory_order_relaxed)->capacity();
    }

    /**
    @brief inserts an item to the queue

    Only the owner thread can insert an item to the queue.
    The operation can trigger the queue to resize its capacity
    if more space is required.

    @tparam O data type

    @param item the item to perfect-forward to the queue
    */
    inline constexpr void //
    push(auto&& item)
        requires std::is_constructible_v<T, decltype(item)>
    {
        ssize_t b = back_.load(std::memory_order_relaxed);
        ssize_t f = front_.load(std::memory_order_acquire);
        block* blk = block_.load(std::memory_order_relaxed);

        // queue is full
        if (blk->capacity() - 1 < (b - f))
        {
            block* tmp = blk->expand(f, b);
            garbage_.push_back(blk);
            std::swap(blk, tmp);
            block_.store(blk, std::memory_order_relaxed);
        }

        blk->set(b, std::forward<decltype(item)>(item));
        std::atomic_thread_fence(std::memory_order_release);
        back_.store(b + 1, std::memory_order_relaxed);
    }

    /**
    @brief pops out an item from the queue

    Only the owner thread can pop out an item from the queue.
    The return can be a @std_nullopt if this operation failed (empty queue).
    */
    inline constexpr bool //
    pop(T& dest)
    {
        bool success = true;
        ssize_t b = back_.load(std::memory_order_relaxed) - 1;
        block* blk = block_.load(std::memory_order_relaxed);
        back_.store(b, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        ssize_t f = front_.load(std::memory_order_relaxed);

        if (f <= b)
        {
            dest = blk->load(b);
            if (f == b)
            {
                // the last item just got stolen
                if (!front_.compare_exchange_strong(f, f + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    success = false;
                }
                back_.store(b + 1, std::memory_order_relaxed);
            }
        }
        else
        {
            back_.store(b + 1, std::memory_order_relaxed);
        }

        return success;
    }

    /**
    @brief steals an item from the queue

    Any threads can try to steal an item from the queue.
    The return can be a @std_nullopt if this operation failed (not necessary empty).
    */
    inline constexpr bool //
    steal(T& dest)
    {
        ssize_t f = front_.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        ssize_t b = back_.load(std::memory_order_acquire);

        if (f < b)
        {
            block* blk = block_.load(std::memory_order_consume);
            dest = blk->load(f);
            if (!front_.compare_exchange_strong(f, f + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                return false;
            }
        }

        return true;
    }
};

#endif // FIBER_WSQ_HXX
