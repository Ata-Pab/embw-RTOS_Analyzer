#pragma once

#include <QDateTime>
#include <QString>

// ---------------------------------------------------------------------------
// Event codes match the Python log_listener.py EVENT_MAP:
//   0=INITIALIZED  1=PUSHED(RE-INIT)  2=SWITCHED IN  3=SWITCHED OUT
//   4=TERMINATED   5=SUSPENDED        23=STACK OVERFLOW
// Message wire format: "name;id;eventCode"
// ---------------------------------------------------------------------------

enum class RtosEventType : int
{
    Initialized = 0,
    PushedReInit = 1,
    SwitchedIn = 2,
    SwitchedOut = 3,
    Terminated = 4,
    Suspended = 5,
    StackOverflow = 23,
    Unknown = -1
};

inline QString rtosEventName(RtosEventType t)
{
    switch (t)
    {
    case RtosEventType::Initialized:
        return QStringLiteral("INITIALIZED");
    case RtosEventType::PushedReInit:
        return QStringLiteral("PUSHED (RE-INIT)");
    case RtosEventType::SwitchedIn:
        return QStringLiteral("SWITCHED IN");
    case RtosEventType::SwitchedOut:
        return QStringLiteral("SWITCHED OUT");
    case RtosEventType::Terminated:
        return QStringLiteral("TERMINATED");
    case RtosEventType::Suspended:
        return QStringLiteral("SUSPENDED");
    case RtosEventType::StackOverflow:
        return QStringLiteral("STACK OVERFLOW");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

inline RtosEventType rtosEventFromCode(int code)
{
    switch (code)
    {
    case 0:
        return RtosEventType::Initialized;
    case 1:
        return RtosEventType::PushedReInit;
    case 2:
        return RtosEventType::SwitchedIn;
    case 3:
        return RtosEventType::SwitchedOut;
    case 4:
        return RtosEventType::Terminated;
    case 5:
        return RtosEventType::Suspended;
    case 23:
        return RtosEventType::StackOverflow;
    default:
        return RtosEventType::Unknown;
    }
}

struct RtosEvent
{
    QDateTime timestamp;
    QString taskName;
    int taskId{0};
    RtosEventType eventType{RtosEventType::Unknown};
    QString rawLine; ///< pre-formatted display string
};
