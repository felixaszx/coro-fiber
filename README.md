# `coro-fiber`
A C++ coroutine library that provides a feel of fiber (user-land threading).

## How does this work?
The idea, in its essense, is to co-operatively switch between different corountines in a single thread (multi-threaded version is under its way).

## What is a fiber?
A fiber is often referred as a user-land thread or a stackful coroutine. It provides partial context switching compare to a thread to minimize its cost.
The problem of fiber is its stackful context. When switching between fibers, the program needs to reset lots of CPU registers and flags. Addtionally, that also makes debugging tools like Valgrind struggle to figure out what happened before and after switching. But, that's not to say fiber is not good with its performance. In fact a well optimized fiber library can have wonderful performance-metrics that matches nested function calls.

## What is `C++20` coroutine
The commitee has finally decided to add support to stackless corountine in `C++20` after years of arguing. C++20 coroutine is a function that allows pause-resume with its context store in the heap. In pratice, it's nothing more than calling some functions. This is fast, very fast `:/`.

## Details of `coro-fiber`
`coro-fiber` is an educational project for experimenting `C++20` corountines. The whole point of this project is to test if it is possible to provide a `std::thread` like experence using coroutine.

Each thread has a dedicated `thread_local` global context. When a `fiber` is created, it will be added to the ready queue of the thread it was created on. Since this is co-operative, fibers in the ready queue will not be executed unless the currently running fiber decides to pause.

```c++
// The main_fiber
fbc::fiber_coro //
main_fiber(std::span<const char*> args)
{
    co_return 0;
}

// Setup the thread context in the main thread
int //
main(int argc, const char** argv)
{
    fbc::fiber main_fb(main_fiber, std::span<const char*>(argv, argc));
    while (fbc::this_thread::fiber_thr_running())
    {
    }
    return main_fb.coro().promise().get_return();
}
```
When `main_fiber` returns and the ready queue of the main thread is empty, main function returns. A fiber is very similar to `std::thread`, except its executing function must return `fbc::fiber_coro`. A fiber can return an integer, but at the current stage its return values are discarded.

After starting the context and the main fiber of a thread, the main fiber function should be the new main function. Inside the new main function, create a new fiber as below:
```c++
// The main_fiber
fbc::fiber_coro //
main_fiber(std::span<const char*> args)
{
    static auto cc_func = []() -> fbc::fiber_coro
    {
        std::println("co_return from fiber cc");
        co_return 0;
    };

    fbc::fiber cc(cc_func);

    std::println("co_return from main fiber");
    co_return 0;
}
```
The context will not execute `cc_func` until co-operatively asked to, or the any fibers before `cc` is terminated. Once main_fiber is terminated, the context will switch to `cc`.
```
$ ./a.out
co_return from main fiber
co_return from fiber cc
```
The main fiber can yield to allow `cc` to be executed.
```c++
// The main_fiber
fbc::fiber_coro //
main_fiber(std::span<const char*> args)
{
    static auto cc_func = []() -> fbc::fiber_coro
    {
        std::println("co_return from fiber cc");
        co_return 0;
    };

    fbc::fiber cc(cc_func);
    co_yield fbc::yield_for::reschedule;

    std::println("co_return from main fiber");
    co_return 0;
}
```
`co_yield` is always used in scheduling operations.
- `fbc::yield_for::reschedule`: place the calling fiber at the end of ready queue
- `fbc::yield_for::join`: more on that later
- `fbc::yield_for::noop`: do nothing and continue the calling fiber

A fiber can wait for other fiber to join
```c++
// The main_fiber
fbc::fiber_coro //
main_fiber(std::span<const char*> args)
{
    static auto cc_func = []() -> fbc::fiber_coro
    {
        std::println("co_return from fiber cc");
        co_return 0;
    };

    fbc::fiber cc(cc_func);
    co_yield cc.join();

    std::println("co_return from main fiber");
    co_return 0;
}
```
In this case, the calling fiber will be removed from the ready queue until the `cc` terminates. Once `cc` is done, all fibers that are waiting for it will be placed in the front of the ready queue. `co_yield fbc::yield_for::join` is the scheduling command used here, but calling it directly will result in `co_yield fbc::yield_for::noop` because the context has no knowledge about the fiber waiting for. With join, the execution output will be like that:
```
$ ./a.out
co_return from fiber cc
co_return from main fiber
```

On the other side, `mutex` is a sync operation. For all sync operations, use `co_await` instead.
```c++
fbc::mutex mtx;

// The main_fiber
fbc::fiber_coro //
main_fiber(std::span<const char*> args)
{
    static auto cc_func = []() -> fbc::fiber_coro
    {
        co_await mtx.lock();
        std::println("co_return from fiber cc");
        mtx.unlock();
        co_return 0;
    };

    fbc::fiber cc(cc_func);
    co_await mtx.lock();
    std::println("co_return from main fiber");
    mtx.unlock();
    co_return 0;
}
```
If the `mutex` is unlocked, then `co_await` will lock the `mutex` and continues. If it was locked, then `co_await` will place the calling fiber inside the waiting queue of the locking `mutex`. Once the fiber owns the lock unlocked it, the first fiber inside the waiting queue will be pushed to the back of the ready queue.
