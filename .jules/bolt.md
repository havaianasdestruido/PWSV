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

## 2024-11-20 - String lookup overhead in Audio thread
**Learning:** `apvts_->getRawParameterValue("...")` performs a string/map lookup and was called continuously inside the `processBlock` real-time audio thread. This can cause severe CPU spikes and audio dropouts because string comparisons and locking map lookups are slow and not real-time safe.
**Action:** Always cache `std::atomic<float>*` parameter pointers from `juce::AudioProcessorValueTreeState` in `prepareToPlay` (to ensure the APVTS is fully constructed) and only read from the cached atomic pointers inside `processBlock`.

## 2024-05-24 - Pre-allocating WebSocket buffer
**Learning:** Re-allocating a vector buffer dynamically for each broadcast on a WebSocket server can create unnecessary allocations, fragmenting the heap and affecting real-time performance.
**Action:** Replace dynamic local vector with a pre-allocated class member (`std::vector::clear()` and `reserve()`) in methods called heavily, like `WebSocketServer::sendToAll()`.

## 2024-05-24 - Guard real-time data structures
**Learning:** Hard limits are needed for data structures manipulated in real-time loops like `processBlock`, such as unbounded push_back for MIDI notes which could eventually overflow limits and trigger dynamic allocation.
**Action:** Always add size guards before pushing into vectors in critical realtime paths.

## 2024-06-25 - Avoid Mutex Lock Contention in Real-Time Threads
**Learning:** Polling loops in background threads (like the WebSocket server) that repeatedly check a condition by locking a shared mutex can cause severe lock contention, increasing the chance of `try_lock()` failures in the real-time audio thread, leading to dropped frames or state updates.
**Action:** Extract frequently checked state variables (like `ppq` and `isPlaying`) into `std::atomic` variables. Update these lock-free atomics unconditionally on the real-time thread, and use them on the polling thread to decide if the heavier payload structure actually needs to be locked and sent.

## 2024-11-20 - Unconditional atomic store overhead in Audio thread
**Learning:** Unconditional atomic stores (`store()`) invalidate cache lines for other cores, leading to "cache line bouncing" and performance degradation, even when the value hasn't changed. This is especially true for rarely changing values (like UI parameters or boolean states) in high-frequency real-time contexts like `processBlock`.
**Action:** Wrap `std::atomic::store()` with a `load(std::memory_order_relaxed)` check if the value rarely changes. This prevents measurable overhead from unconditional memory synchronization barriers.
