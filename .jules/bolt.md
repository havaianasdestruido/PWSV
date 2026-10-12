## 2024-11-20 - O(N) array removal on audio thread
**Learning:** `processBlock` iterates through MIDI messages and previously removed them with an `std::vector::erase` loop, which is an O(N) operation due to shifting all elements, and shouldn't be done on a real-time thread anyway. We can optimize it to `O(1)` by swapping with the last element. Also the vector allocation itself was delayed to first note which does memory allocation on real-time thread which is bad, so I fixed that too via `reserve`.
**Action:** Found O(N) erase in an audio callback, optimized to O(1) swap-and-pop, pre-allocated the vector.

## 2024-11-20 - String concatenation overhead in JSON serialization
**Learning:** `WebSocketServer::buildJson()` was using many `juce::String` concatenations (`+`) which internally allocates and copies memory for every operation. This was happening on the WebSocket send thread periodically. Using a single `juce::String` with `preallocateBytes` and `operator<<` drastically reduces heap allocations and copying overhead.
**Action:** Identified redundant heap string concatenations, replaced with pre-allocated `juce::String` and `operator<<`.
