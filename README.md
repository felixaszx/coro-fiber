# `coro-fiber`
A C++ coroutine library that provides a feel of fiber (user-land threading).

## How does this work?
The idea, in its essense, is to co-operatively switch between different corountines in a single thread (multi-threaded version is now ready!).

## Document
See main.cxx for an example

## Third-party
Thanks to taskflow and his work steal queue https://github.com/taskflow/work-stealing-queue?tab=License-1-ov-file#readme

## Some issues
On larget lambda captures, the performance can drop significantly. Apparently the idea of fiber + scheduler only works well on computational intense environment.

My original goal is to create a base framework for my game engine, but I now realise that sometimes a much simpler design will be good (LOL).

## Next steps
More simplify the async framework. Try to *avoid* coroutine.
