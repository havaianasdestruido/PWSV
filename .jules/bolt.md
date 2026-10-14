## 2024-11-20 - O(N) array removal on audio thread
**Learning:** `processBlock` iterates through MIDI messages and previously removed them with an `std::vector::erase` loop, which is an O(N) operation due to shifting all elements, and shouldn't be done on a real-time thread anyway. We can optimize it to `O(1)` by swapping with the last element. Also the vector allocation itself was delayed to first note which does memory allocation on real-time thread which is bad, so I fixed that too via `reserve`.
**Action:** Found O(N) erase in an audio callback, optimized to O(1) swap-and-pop, pre-allocated the vector.

## 2024-11-20 - String concatenation overhead in JSON serialization
**Learning:** `WebSocketServer::buildJson()` was using many `juce::String` concatenations (`+`) which internally allocates and copies memory for every operation. This was happening on the WebSocket send thread periodically. Using a single `juce::String` with `preallocateBytes` and `operator<<` drastically reduces heap allocations and copying overhead.
**Action:** Identified redundant heap string concatenations, replaced with pre-allocated `juce::String` and `operator<<`.
## 2024-05-24 - Nagle's Algorithm latency in WebSocket Framing
**Learning:** Sending the WebSocket header and payload in two separate `send()` calls without `TCP_NODELAY` triggers Nagle's algorithm, causing the payload to be delayed by up to 200ms while waiting for the header's ACK.
**Action:** Always batch small protocol headers with their payloads into a single `send()` buffer or use `writev` / set `TCP_NODELAY`.

## 2024-05-24 - Redundant Mutex Locks in Tick Loops
**Learning:** Calling a state-fetching function (`getPosition()`) multiple times within a single tick sequence (`shouldSend`, `buildJson`, `afterSend`) acquires the mutex and copies a 1.5KB struct 3 times per loop, increasing audio-thread contention risk.
**Action:** Fetch thread-safe state exactly once per loop iteration and pass it by const reference to consumer functions.

## 2024-05-18 - Audio Thread Non-Blocking Telemetry Lock
**Learning:** A standard `std::lock_guard` used for updating telemetry data in the `processBlock` audio thread creates a risk of priority inversion and audio dropouts if the background WebSocket/telemetry thread holds the lock for reading.
**Action:** When updating continuous, high-frequency state from an audio thread to a background thread, using `try_lock()` and simply dropping the update if the lock is held is a safe, lock-free alternative because the telemetry consumer operates at a much lower refresh rate than the audio buffer size.
