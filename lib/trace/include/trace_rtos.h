#ifndef TRACE_RTOS_H
#define TRACE_RTOS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /* --------------------------------------------------------------------------
     * Event types
     * --------------------------------------------------------------------------*/
    typedef enum
    {
        TRACE_EVT_TASK_SWITCH_IN = 0,
        TRACE_EVT_TASK_SWITCH_OUT,
        TRACE_EVT_TASK_CREATE,
        TRACE_EVT_TASK_DELETE,
        TRACE_EVT_ISR_ENTER,
        TRACE_EVT_ISR_EXIT,
        TRACE_EVT_QUEUE_SEND,
        TRACE_EVT_QUEUE_RECEIVE,
        TRACE_EVT_SEM_TAKE,
        TRACE_EVT_SEM_GIVE,
        TRACE_EVT_SYSTEM,
        TRACE_EVT_COUNT
    } TraceEventType;

    /* --------------------------------------------------------------------------
     * Single recorded event
     * --------------------------------------------------------------------------*/
    typedef struct
    {
        TraceEventType type;
        uint32_t timestamp_us; /* Simulated microsecond timestamp      */
        uint32_t id;           /* Task/ISR/Queue/Semaphore handle ID   */
        const char *name;      /* Human-readable label (may be NULL)   */
        uint32_t param;        /* Optional extra parameter             */
    } TraceEvent;

    /* --------------------------------------------------------------------------
     * Callback invoked each time an event is recorded.
     * Set to NULL to disable.
     * --------------------------------------------------------------------------*/
    typedef void (*TraceCallback)(const TraceEvent *event, void *user_data);

    /** Initialise the trace module.  Must be called before any other function. */
    void trace_init(void);

    /** Register a callback that is called for every recorded event. */
    void trace_set_callback(TraceCallback cb, void *user_data);

    /** Record a single event.  Thread-safe under simulation (single-threaded). */
    void trace_record(TraceEventType type, uint32_t id,
                      const char *name, uint32_t param,
                      uint32_t timestamp_us);

    /** Return the number of events recorded since the last trace_clear(). */
    uint32_t trace_event_count(void);

    /** Access a recorded event by index (0-based).  Returns NULL if out of range. */
    const TraceEvent *trace_get_event(uint32_t index);

    /** Clear all recorded events. */
    void trace_clear(void);

/* --------------------------------------------------------------------------
 * Hook macros
 * --------------------------------------------------------------------------*/
#define TRACE_TASK_SWITCH_IN(id, name, ts) trace_record(TRACE_EVT_TASK_SWITCH_IN, (id), (name), 0, (ts))
#define TRACE_TASK_SWITCH_OUT(id, name, ts) trace_record(TRACE_EVT_TASK_SWITCH_OUT, (id), (name), 0, (ts))
#define TRACE_TASK_CREATE(id, name, ts) trace_record(TRACE_EVT_TASK_CREATE, (id), (name), 0, (ts))
#define TRACE_TASK_DELETE(id, name, ts) trace_record(TRACE_EVT_TASK_DELETE, (id), (name), 0, (ts))
#define TRACE_ISR_ENTER(id, name, ts) trace_record(TRACE_EVT_ISR_ENTER, (id), (name), 0, (ts))
#define TRACE_ISR_EXIT(id, name, ts) trace_record(TRACE_EVT_ISR_EXIT, (id), (name), 0, (ts))
#define TRACE_QUEUE_SEND(id, name, ts) trace_record(TRACE_EVT_QUEUE_SEND, (id), (name), 0, (ts))
#define TRACE_QUEUE_RECEIVE(id, name, ts) trace_record(TRACE_EVT_QUEUE_RECEIVE, (id), (name), 0, (ts))
#define TRACE_SEM_TAKE(id, name, ts) trace_record(TRACE_EVT_SEM_TAKE, (id), (name), 0, (ts))
#define TRACE_SEM_GIVE(id, name, ts) trace_record(TRACE_EVT_SEM_GIVE, (id), (name), 0, (ts))
#define TRACE_SYSTEM(id, name, param, ts) trace_record(TRACE_EVT_SYSTEM, (id), (name), (param), (ts))

#ifdef __cplusplus
}
#endif

#endif /* TRACE_RTOS_H */
