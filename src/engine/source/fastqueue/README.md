# FastQueue — Concurrent Queue Library

## Overview

`fastqueue` provides thread-safe, bounded queue implementations for high-throughput inter-thread communication in the GuardSarm Engine. It offers two implementations behind a common `IQueue<T>` interface:

- **`CQueue`** — Lock-free queue backed by `moodycamel::BlockingConcurrentQueue` with optimized block/index traits. Best for high-contention scenarios.
- **`StdQueue`** — Mutex-based queue backed by `std::queue` with condition variable. Simpler, with exact (not approximate) size reporting.

Both implementations support optional **token-bucket rate limiting** on dequeue operations and **bulk pop** for batch processing.

## Architecture

```
                         IQueue<T>
                        (interface)
                       ┌─────┴─────┐
                       │           │
                  CQueue<T>    StdQueue<T>
                  (lock-free)  (mutex-based)
                       │           │
                       └─────┬─────┘
                             │
                      ┌──────┴──────┐
                      │ RateLimiter │  (optional, token bucket)
                      └─────────────┘

    Producer ──push()/tryPush()──► Queue ──waitPop()/tryPop()──► Consumer
                                    │
                                    ├── tryPopBulk() ──► Batch consumer
                                    │
                                    └── aproxFreeSlots() ──► Backpressure
```

### Comparison

| Feature | CQueue | StdQueue |
|---------|--------|----------|
| Synchronization | Lock-free (atomics) | Mutex + condition variable |
| Underlying storage | `moodycamel::BlockingConcurrentQueue` | `std::queue` |
| Size reporting | Approximate | Exact |
| Bulk dequeue | Native (`try_dequeue_bulk`) | Loop-based |
| Best for | High contention, MPMC | Low contention, simple use cases |

## Key Concepts

### Bounded Capacity

Both implementations enforce a minimum capacity of **8,192 elements** (`MIN_QUEUE_CAPACITY`). For `CQueue`, this is 2× the block size (512) to ensure optimal performance. Recommended sizes: $2^{17}$ (131K), $2^{20}$ (1M).

### Token-Bucket Rate Limiting

An optional `RateLimiter` can throttle dequeue rate (elements per second). It uses a lock-free token bucket algorithm:

- Tokens refill at a constant rate based on elapsed time
- `tryAcquire(n)` — non-blocking, returns `false` if insufficient tokens
- `waitAcquire(n, timeout)` — sleeps efficiently until tokens are available or timeout expires
- Applied transparently on `waitPop()`, `tryPop()`, and `tryPopBulk()`

Constructed via: `CQueue(capacity, maxElementsPerSecond, burstSize)`.

### CQueue Traits

`CQueue` uses custom `WQueueTraits` optimized for large queues:

- `BLOCK_SIZE = 512` — fewer allocations, better cache locality
- `IMPLICIT_INITIAL_INDEX_SIZE = 512` — covers up to ~2M elements without index reallocation

## Directory Structure

```
fastqueue/
├── CMakeLists.txt
├── interface/fastqueue/
│   └── iqueue.hpp                    # IQueue<T> — common interface
├── include/fastqueue/
│   ├── cqueue.hpp                    # CQueue<T> — lock-free implementation
│   ├── stdqueue.hpp                  # StdQueue<T> — mutex-based implementation
│   └── ratelimiter.hpp               # RateLimiter — token bucket algorithm
├── src/
│   └── cqueue.cpp                    # CQueue template instantiation (empty body)
├── test/
│   ├── mocks/queue/
│   │   └── mockQueue.hpp             # GMock of IQueue<T>
│   └── src/component/
│       ├── cqueue_test.cpp           # CQueue component tests
│       └── stdqueue_test.cpp         # StdQueue component tests
└── benchmark/
    ├── README.md                     # Benchmark documentation
    └── src/
        ├── cqueue_bench.cpp          # CQueue benchmarks
        ├── stdqueue_bench.cpp        # StdQueue benchmarks
        └── comparison_bench.cpp      # Head-to-head comparison
```

## Public Interface

### `IQueue<T>` (iqueue.hpp)

```cpp
namespace fastqueue {
template<typename T>
class IQueue {
    virtual bool push(T&& element) = 0;                          // Move-push, false if full
    virtual bool tryPush(const T& element) = 0;                  // Copy-push, false if full
    virtual bool waitPop(T& element, int64_t timeout) = 0;       // Blocking pop (timeout in µs)
    virtual bool tryPop(T& element) = 0;                         // Non-blocking pop
    virtual bool empty() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual std::size_t aproxFreeSlots() const noexcept = 0;     // Backpressure signal
    virtual std::size_t tryPopBulk(T* elements, std::size_t max) = 0;  // Batch pop
};
}
```

### `CQueue<T>` (cqueue.hpp)

```cpp
explicit CQueue(int capacity);                                         // Basic
CQueue(int capacity, double maxElementsPerSecond, double burstSize);   // With rate limiting
```

### `StdQueue<T>` (stdqueue.hpp)

```cpp
explicit StdQueue(int capacity);                                       // Basic
StdQueue(int capacity, double maxElementsPerSecond, double burstSize); // With rate limiting
```

### `RateLimiter` (ratelimiter.hpp)

```cpp
RateLimiter(size_t maxElementsPerSecond, size_t burstSize = 0);
bool tryAcquire(size_t count = 1);                    // Non-blocking
bool waitAcquire(size_t count, int64_t timeoutMicros); // Blocking with timeout
```

## Implementation Details

### CQueue

- Wraps `moodycamel::BlockingConcurrentQueue<T, WQueueTraits>`.
- `push()` / `tryPush()` → `try_enqueue()` (never allocates beyond initial capacity).
- `waitPop()` → `wait_dequeue_timed()` with microsecond timeout.
- `tryPopBulk()` → `try_dequeue_bulk()` (native batch operation).
- Size reporting is approximate (`size_approx()`).

### StdQueue

- Wraps `std::queue<T>` with `std::mutex` + `std::condition_variable`.
- `push()` acquires lock, checks capacity, pushes, notifies condition variable.
- `waitPop()` uses `condition_variable::wait_for()` for timed blocking.
- `tryPopBulk()` implemented as a loop under a single lock acquisition.
- Size reporting is exact.

### Rate Limiter Integration

When a `RateLimiter` is configured, dequeue operations follow a two-phase approach:

1. **Acquire tokens** from the rate limiter (may wait or fail)
2. **Dequeue** from the underlying queue with remaining timeout

The rate limiter uses `std::atomic<double>` for tokens and `std::atomic<int64_t>` for the last refill timestamp, achieving lock-free operation. Token refill is computed lazily on each access based on elapsed microseconds.

## CMake Targets

| Target | Type | Alias | Description |
|--------|------|-------|-------------|
| `fastqueue_ifastqueue` | INTERFACE | `fastqueue::ifastqueue` | `IQueue<T>` interface |
| `fastqueue_fasqueue` | STATIC | `fastqueue::fastqueue` | Both implementations (links `moodycamel::concurrentqueue`) |
| `fastqueue_mocks` | INTERFACE | `fastqueue::mocks` | GMock `MockQueue<T>` |
| `fastqueue_ctest` | Executable | — | Component tests |
| `fastqueue_benchmark` | Executable | — | CQueue benchmarks |
| `fastqueue_stdqueue_benchmark` | Executable | — | StdQueue benchmarks |
| `fastqueue_comparison_benchmark` | Executable | — | Head-to-head comparison |

## Testing

- **Component tests** (`cqueue_test.cpp`, `stdqueue_test.cpp`): Test SPSC/MPSC/SPMC/MPMC patterns, capacity bounds, rate limiting behavior.
- **Benchmarks**: SPSC, MPSC, SPMC, MPMC, bulk operations, high contention, and rate limiting scenarios with configurable thread counts and queue sizes. See [benchmark/README.md](benchmark/README.md).

## Consumers

| Consumer | Dependency | Usage |
|----------|------------|-------|
| **router** | `fastqueue::ifastqueue` | Worker threads use `IQueue<T>` as the event ingestion queue between the orchestrator and routing workers |
| **streamlog** | `fastqueue::fastqueue` | Async logging channel uses `StdQueue` to buffer log messages for background flushing |
| **main.cpp** | `fastqueue::fastqueue` | Creates `CQueue` and `StdQueue` instances for the engine's event pipeline |
