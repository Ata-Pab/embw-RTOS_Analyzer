#include "trace_rtos.h"
#include <stddef.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Internal storage
 * --------------------------------------------------------------------------*/
#define TRACE_MAX_EVENTS 1024

static TraceEvent   s_events[TRACE_MAX_EVENTS];
static uint32_t     s_count     = 0;
static TraceCallback s_callback  = NULL;
static void        *s_user_data = NULL;

/* --------------------------------------------------------------------------
 * API implementation
 * --------------------------------------------------------------------------*/

void trace_init(void)
{
    s_count     = 0;
    s_callback  = NULL;
    s_user_data = NULL;
    memset(s_events, 0, sizeof(s_events));
}

void trace_set_callback(TraceCallback cb, void *user_data)
{
    s_callback  = cb;
    s_user_data = user_data;
}

void trace_record(TraceEventType type, uint32_t id,
                  const char *name, uint32_t param,
                  uint32_t timestamp_us)
{
    if (s_count >= TRACE_MAX_EVENTS) {
        return; /* Buffer full — caller should call trace_clear() */
    }

    TraceEvent *ev = &s_events[s_count++];
    ev->type         = type;
    ev->timestamp_us = timestamp_us;
    ev->id           = id;
    ev->name         = name;
    ev->param        = param;

    if (s_callback != NULL) {
        s_callback(ev, s_user_data);
    }
}

uint32_t trace_event_count(void)
{
    return s_count;
}

const TraceEvent *trace_get_event(uint32_t index)
{
    if (index >= s_count) {
        return NULL;
    }
    return &s_events[index];
}

void trace_clear(void)
{
    s_count = 0;
}
