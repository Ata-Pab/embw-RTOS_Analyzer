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

```c
struct TraceEvent {
    uint32_t timestamp; // Timestamp of the event
    uint32_t eventType; // Type of the event (e.g., task switch, ISR entry)
    uint32_t taskId;    // ID of the task involved in the event
    uint32_t additionalInfo; // Additional information related to the event - metadata
};
```

## Example Event Types

```c
enum EventType {
    TASK_SWITCH_IN,
    TASK_SWITCH_OUT,
    QUEUE_SEND,
    QUEUE_RECEIVE,
    SEMAPHORE_TAKE,
    SEMAPHORE_GIVE,
    ISR_ENTRY,
    ISR_EXIT
};
```

## Adding Hooks in RTOS
To capture trace data, you can add hooks in the RTOS kernel. For example, in FreeRTOS, you can define trace macros in `FreeRTOSConfig.h`:

```c
#define traceTASK_SWITCHED_IN()  trace_task_switched_in(pxCurrentTCB)
#define traceTASK_SWITCHED_OUT() trace_task_switched_out(pxCurrentTCB)
```

Then, implement the corresponding functions to record the trace data:

```c
void trace_task_switched_in(TaskHandle_t xTask) {
    TraceEvent event;
    event.timestamp = get_current_time();
    event.eventType = TASK_SWITCH_IN;
    event.taskId = (uint32_t)xTask;
    // Store the event in a buffer or send it to the host
    log_trace_event(event);
}

void trace_task_switched_out(TaskHandle_t xTask) {
    TraceEvent event;
    event.timestamp = get_current_time();
    event.eventType = TASK_SWITCH_OUT;
    event.taskId = (uint32_t)xTask;
    // Store the event in a buffer or send it to the host
    log_trace_event(event);
}
```