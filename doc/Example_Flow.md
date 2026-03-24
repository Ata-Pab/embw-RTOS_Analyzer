## Example Flow of Trace Data

```
    [RTOS Kernel]
          |
    (trace macro triggered) <------------------ #define traceTASK_SWITCHED_IN()  trace_task_switched_in(pxCurrentTCB) ...
          |
    [trace recorder] <------------------ trace_task_switched_in()
          |
    (recorded trace data)
          |
[event stored in Buffer] <------------------ trace_task_switched_in()
          |
    (trace data sent to host)
          |
    [Host PC] Tool Visualization

```

## Example Struct to Store Trace Data

```cpp
struct TraceEvent {
    uint32_t timestamp; // Timestamp of the event
    uint32_t eventType; // Type of the event (e.g., task switch, ISR entry)
    uint32_t taskId;    // ID of the task involved in the event
    uint32_t additionalInfo; // Additional information related to the event - metadata
};
```