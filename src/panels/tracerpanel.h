#pragma once

#include <QWidget>
#include <QList>
#include <QColor>
#include "rtos/rtosevent.h"

class QScrollBar;
class QTimer;

// ---------------------------------------------------------------------------
// Internal data structures
// ---------------------------------------------------------------------------

struct TaskTrack
{
    struct ActiveRange
    {
        qint64 startMs;
        qint64 endMs;
    }; ///< endMs=-1 → still running
    struct Marker
    {
        qint64 timeMs;
        RtosEventType type;
    };

    QString name;
    int id{0};
    QColor color;
    QList<ActiveRange> ranges;
    QList<Marker> markers;
    qint64 activeStart{-1}; ///< wall-clock ms at last SWITCHED IN
};

// ---------------------------------------------------------------------------

/**
 * @class TracerPanel
 * @brief Custom painter widget that draws an RTOS task-timeline (Gantt-style).
 *
 * - Each task occupies one horizontal lane.
 * - Green/colored bars represent SWITCHED IN → SWITCHED OUT periods.
 * - Diamond markers represent one-shot events (INIT, PUSH, TERM, SUSP, STACK OVF).
 * - Horizontal axis = elapsed time in ms.  Ctrl+Wheel → zoom.  Wheel → scroll.
 */
class TracerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TracerPanel(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void appendEvent(const RtosEvent &event);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int trackIndexOf(const QString &name, int id);
    void updateScrollBar();

    void drawLegend(QPainter &p, const QRect &area);
    void drawTimeAxis(QPainter &p, const QRect &area, qint64 startMs, qint64 visibleMs);
    void drawTracks(QPainter &p, const QRect &area, qint64 startMs, qint64 visibleMs);

    QList<TaskTrack> m_tracks;
    QScrollBar *m_hScrollBar{nullptr};
    QTimer *m_animTimer{nullptr};

    qint64 m_firstEventMs{-1};
    qint64 m_totalDurationMs{0};
    double m_pixelsPerMs{5.0};
    qint64 m_scrollOffsetMs{0};
    bool m_followLatest{true};

    // Layout constants (px)
    static constexpr int LaneHeight = 42;
    static constexpr int LaneHeaderW = 175;
    static constexpr int TimeAxisH = 26;
    static constexpr int LegendH = 22;
    static constexpr int TopPad = 4;
    static constexpr int BotPad = 4;

    static const QList<QColor> s_palette;
};
