#include "tracerpanel.h"

#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QTimer>
#include <QToolTip>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPolygon>
#include <cmath>

// ─── Colour palette (one colour per task, cycling) ────────────────────────────
const QList<QColor> TracerPanel::s_palette = {
    {0x3B, 0x82, 0xF6}, // blue
    {0x10, 0xB9, 0x81}, // teal
    {0xF5, 0x9E, 0x0B}, // amber
    {0x8B, 0x5C, 0xF6}, // violet
    {0xEC, 0x48, 0x99}, // pink
    {0x22, 0xC5, 0x5E}, // green
    {0xF9, 0x73, 0x16}, // orange
    {0x06, 0xB6, 0xD4}, // cyan
};

// ─── Construction ─────────────────────────────────────────────────────────────
TracerPanel::TracerPanel(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    m_hScrollBar = new QScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setRange(0, 0);
    m_hScrollBar->setStyleSheet(
        "QScrollBar:horizontal { background:#161B22; height:10px; border-radius:5px; }"
        "QScrollBar::handle:horizontal { background:#30363D; border-radius:5px; min-width:20px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }");
    connect(m_hScrollBar, &QScrollBar::valueChanged, this, [this](int val)
            {
        m_scrollOffsetMs = val;
        m_followLatest   = (val == m_hScrollBar->maximum());
        update(); });

    m_vScrollBar = new QScrollBar(Qt::Vertical, this);
    m_vScrollBar->setRange(0, 0);
    m_vScrollBar->setStyleSheet(
        "QScrollBar:vertical { background:#161B22; width:10px; border-radius:5px; }"
        "QScrollBar::handle:vertical { background:#30363D; border-radius:5px; min-height:20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }");
    connect(m_vScrollBar, &QScrollBar::valueChanged, this, [this](int val)
            {
        m_vScrollOffset = val;
        update(); });

    // Blink timer drives the "currently running" bar animation
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(500);
    connect(m_animTimer, &QTimer::timeout, this, [this]
            { update(); });
    m_animTimer->start();
}

// ─── Size hints ───────────────────────────────────────────────────────────────
QSize TracerPanel::sizeHint() const
{
    return QSize(800, 400);
}

QSize TracerPanel::minimumSizeHint() const
{
    return QSize(200, 150);
}

// ─── Track lookup / creation ─────────────────────────────────────────────────
int TracerPanel::trackIndexOf(const QString &name, int id)
{
    for (int i = 0; i < m_tracks.size(); ++i)
        if (m_tracks[i].name == name)
            return i;

    TaskTrack t;
    t.name = name;
    t.id = id;
    t.color = s_palette[m_tracks.size() % s_palette.size()];
    m_tracks.append(t);
    updateGeometry(); // notify parent / scroll area
    return m_tracks.size() - 1;
}

// ─── Public slots ─────────────────────────────────────────────────────────────
void TracerPanel::appendEvent(const RtosEvent &event)
{
    if (m_firstEventUs < 0)
        m_firstEventUs = event.timestampUs;

    const qint64 timeUs = event.timestampUs - m_firstEventUs;
    m_totalDurationMs = qMax(m_totalDurationMs, timeUs / 1000);

    const int idx = trackIndexOf(event.taskName, event.taskId);
    TaskTrack &track = m_tracks[idx];
    track.id = event.taskId;

    switch (event.eventType)
    {
    case RtosEventType::SwitchedIn:
        track.activeStart = timeUs;
        break;

    case RtosEventType::SwitchedOut:
        if (track.activeStart >= 0)
        {
            track.ranges.append({track.activeStart, timeUs});
            track.activeStart = -1;
        }
        break;

    case RtosEventType::Terminated:
    case RtosEventType::StackOverflow:
        if (track.activeStart >= 0)
        {
            track.ranges.append({track.activeStart, timeUs});
            track.activeStart = -1;
        }
        track.markers.append({timeUs, event.eventType});
        break;

    default:
        track.markers.append({timeUs, event.eventType});
        break;
    }

    updateScrollBar();

    // Auto-follow: keep the latest data in view
    if (m_followLatest)
    {
        const int sbW = m_vScrollBar->isVisible() ? m_vScrollBar->width() : 0;
        const int plotW = width() - LaneHeaderW - sbW;
        if (plotW > 0)
        {
            const qint64 visibleMs = static_cast<qint64>(plotW / m_pixelsPerMs);
            const qint64 target = qMax<qint64>(0, m_totalDurationMs - visibleMs + 100);
            m_scrollOffsetMs = target;
            m_hScrollBar->blockSignals(true);
            m_hScrollBar->setValue(static_cast<int>(m_scrollOffsetMs));
            m_hScrollBar->blockSignals(false);
        }
    }
    update();
}

void TracerPanel::clear()
{
    m_tracks.clear();
    m_firstEventUs = -1;
    m_totalDurationMs = 0;
    m_scrollOffsetMs = 0;
    m_vScrollOffset = 0;
    m_followLatest = true;
    m_hScrollBar->setRange(0, 0);
    m_vScrollBar->setRange(0, 0);
    updateGeometry();
    update();
}

// ─── Resize / scrollbar ───────────────────────────────────────────────────────
void TracerPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int sbH = m_hScrollBar->sizeHint().height();
    const int sbW = m_vScrollBar->sizeHint().width();
    m_hScrollBar->setGeometry(LaneHeaderW,
                              height() - sbH - BotPad,
                              width() - LaneHeaderW - sbW,
                              sbH);
    m_vScrollBar->setGeometry(width() - sbW,
                              TopPad + LegendH + TimeAxisH,
                              sbW,
                              height() - TopPad - LegendH - TimeAxisH - BotPad - sbH);
    updateScrollBar();
}

void TracerPanel::updateScrollBar()
{
    const int sbW = m_vScrollBar->isVisible() ? m_vScrollBar->width() : 0;
    const int plotW = width() - LaneHeaderW - sbW;
    if (plotW <= 0)
        return;

    const qint64 visibleMs = static_cast<qint64>(plotW / m_pixelsPerMs);
    const qint64 maxScroll = qMax<qint64>(0, m_totalDurationMs - visibleMs + 200);

    m_hScrollBar->blockSignals(true);
    m_hScrollBar->setRange(0, static_cast<int>(maxScroll));
    m_hScrollBar->setPageStep(static_cast<int>(visibleMs));
    m_hScrollBar->setSingleStep(qMax(1, static_cast<int>(visibleMs / 10)));
    m_hScrollBar->blockSignals(false);

    const int tracksY = TopPad + LegendH + TimeAxisH;
    const int tracksH = height() - tracksY - BotPad - m_hScrollBar->height();
    const int totalTracksH = m_tracks.size() * LaneHeight;

    if (totalTracksH > tracksH && tracksH > 0)
    {
        m_vScrollBar->blockSignals(true);
        m_vScrollBar->setRange(0, totalTracksH - tracksH);
        m_vScrollBar->setPageStep(tracksH);
        m_vScrollBar->setSingleStep(LaneHeight);
        m_vScrollBar->show();
        m_vScrollBar->blockSignals(false);
    }
    else
    {
        m_vScrollBar->blockSignals(true);
        m_vScrollBar->setRange(0, 0);
        m_vScrollBar->hide();
        m_vScrollBar->blockSignals(false);
    }
}

// ─── Wheel ────────────────────────────────────────────────────────────────────
void TracerPanel::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        // Zoom
        const double factor = (event->angleDelta().y() > 0) ? 1.25 : (1.0 / 1.25);
        m_pixelsPerMs = qBound(0.1, m_pixelsPerMs * factor, 200.0);
        updateScrollBar();
        update();
    }
    else
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const int mouseX = event->position().x();
#else
        const int mouseX = event->pos().x();
#endif

        if (mouseX < LaneHeaderW)
        {
            // Vertical scroll
            if (m_vScrollBar->isVisible())
            {
                const int step = m_vScrollBar->singleStep() * 3;
                const int delta = (event->angleDelta().y() > 0) ? -step : step;
                m_vScrollBar->setValue(m_vScrollBar->value() + delta);
            }
        }
        else
        {
            // Horizontal scroll
            const int step = m_hScrollBar->singleStep() * 3;
            const int delta = (event->angleDelta().y() > 0) ? -step : step;
            const int newVal = qBound(m_hScrollBar->minimum(),
                                      m_hScrollBar->value() + delta,
                                      m_hScrollBar->maximum());
            m_hScrollBar->setValue(newVal); // triggers valueChanged → update
        }
    }
    event->accept();
}

// ─── Paint ────────────────────────────────────────────────────────────────────
void TracerPanel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int sbH = m_hScrollBar->sizeHint().height();
    const int sbW = m_vScrollBar->isVisible() ? m_vScrollBar->width() : 0;
    const int fullH = height() - sbH - BotPad; // drawable area above scrollbar

    // Overall background
    p.fillRect(rect(), QColor(0x0D, 0x11, 0x17));

    // Left header column background
    p.fillRect(0, 0, LaneHeaderW, fullH, QColor(0x16, 0x1B, 0x22));

    const int plotX = LaneHeaderW;
    const int plotW = width() - LaneHeaderW - sbW;
    const int legendY = TopPad;
    const int timeAxisY = legendY + LegendH;
    const int tracksY = timeAxisY + TimeAxisH;
    const int tracksH = fullH - tracksY;

    const qint64 startMs = m_scrollOffsetMs;
    const qint64 visibleMs = (plotW > 0) ? static_cast<qint64>(plotW / m_pixelsPerMs) : 1;

    drawLegend(p, QRect(plotX, legendY, plotW, LegendH));
    drawTimeAxis(p, QRect(plotX, timeAxisY, plotW, TimeAxisH), startMs, visibleMs);

    p.save();
    p.setClipRect(0, tracksY, width() - sbW, tracksH);
    drawTracks(p, QRect(0, tracksY, width() - sbW, tracksH), startMs, visibleMs);
    p.restore();

    // Vertical separator header | plot
    p.setPen(QColor(0x30, 0x3C, 0x50));
    p.drawLine(LaneHeaderW, 0, LaneHeaderW, fullH);
}

// ─── Legend strip ─────────────────────────────────────────────────────────────
void TracerPanel::drawLegend(QPainter &p, const QRect &area)
{
    struct Entry
    {
        const char *label;
        QColor color;
        bool diamond;
    };
    static const QList<Entry> entries = {
        {"RUNNING", {0x3B, 0x82, 0xF6}, false},
        {"INITIALIZED", {0x60, 0xA5, 0xFA}, true},
        {"PUSHED", {0xC0, 0x84, 0xFC}, true},
        {"TERMINATED", {0xF8, 0x71, 0x71}, true},
        {"SUSPENDED", {0xFC, 0xD3, 0x4D}, true},
        {"STK OVERFLOW", {0xFF, 0x33, 0x33}, true},
    };

    p.fillRect(area, QColor(0x16, 0x1B, 0x22));
    p.setPen(QColor(0x30, 0x3C, 0x50));
    p.drawLine(area.x(), area.bottom(), area.right(), area.bottom());

    QFont f;
    f.setPointSize(7);
    p.setFont(f);
    const QFontMetrics fm(f);

    int x = area.x() + 8;
    const int cy = area.y() + area.height() / 2;

    for (const auto &e : entries)
    {
        if (x > area.right() - 40)
            break;
        const QString lbl = QLatin1String(e.label);

        if (e.diamond)
        {
            constexpr int D = 4;
            QPolygon dia;
            dia << QPoint(x + D, cy - D) << QPoint(x + 2 * D, cy)
                << QPoint(x + D, cy + D) << QPoint(x, cy);
            p.setPen(Qt::NoPen);
            p.setBrush(e.color);
            p.drawPolygon(dia);
            x += D * 2 + 4;
        }
        else
        {
            p.setPen(QPen(e.color, 1));
            p.setBrush(e.color.darker(200));
            p.drawRoundedRect(x, cy - 4, 14, 8, 2, 2);
            x += 18;
        }

        p.setPen(QColor(0x9C, 0xA3, 0xAF));
        p.setBrush(Qt::NoBrush);
        const int tw = fm.horizontalAdvance(lbl);
        p.drawText(x, area.y(), tw + 2, area.height(),
                   Qt::AlignLeft | Qt::AlignVCenter, lbl);
        x += tw + 14;
    }

    // Ctrl+Scroll hint (right-aligned)
    const QString hint = QStringLiteral("Ctrl+Scroll = Zoom");
    p.setPen(QColor(0x4B, 0x55, 0x63));
    const int hw = fm.horizontalAdvance(hint);
    p.drawText(area.right() - hw - 8, area.y(), hw + 8, area.height(),
               Qt::AlignLeft | Qt::AlignVCenter, hint);
}

// ─── Time axis ────────────────────────────────────────────────────────────────
void TracerPanel::drawTimeAxis(QPainter &p, const QRect &area,
                               qint64 startMs, qint64 visibleMs)
{
    p.fillRect(area, QColor(0x1C, 0x22, 0x2B));
    p.setPen(QColor(0x30, 0x3C, 0x50));
    p.drawLine(area.x(), area.bottom(), area.right(), area.bottom());

    QFont f;
    f.setPointSize(8);
    p.setFont(f);

    // Pick a "nice" tick interval targeting ~80 px between ticks
    double tickMs = 80.0 / m_pixelsPerMs;
    const double mag = std::pow(10.0, std::floor(std::log10(qMax(1.0, tickMs))));
    const double n = tickMs / mag;
    if (n <= 1.5)
        tickMs = mag;
    else if (n <= 3.5)
        tickMs = 2.0 * mag;
    else if (n <= 7.5)
        tickMs = 5.0 * mag;
    else
        tickMs = 10.0 * mag;
    if (tickMs < 1.0)
        tickMs = 1.0;

    const qint64 tick = static_cast<qint64>(tickMs);
    const qint64 first = (startMs / tick + 1) * tick;

    for (qint64 t = first; t <= startMs + visibleMs + tick; t += tick)
    {
        const int x = area.x() + static_cast<int>((t - startMs) * m_pixelsPerMs);
        if (x < area.x() || x > area.right())
            continue;

        // Tick mark
        p.setPen(QColor(0x4B, 0x55, 0x63));
        p.drawLine(x, area.bottom() - 5, x, area.bottom());

        // Label
        QString label;
        if (t >= 60000)
            label = QStringLiteral("%1m%2s").arg(t / 60000).arg((t % 60000) / 1000);
        else if (t >= 1000)
            label = QStringLiteral("%1.%2s").arg(t / 1000).arg((t % 1000) / 100);
        else
            label = QStringLiteral("%1ms").arg(t);

        p.setPen(QColor(0x6B, 0x72, 0x80));
        p.drawText(x + 3, area.y(), 72, area.height() - 5,
                   Qt::AlignLeft | Qt::AlignVCenter, label);
    }
}

// ─── Task lanes ───────────────────────────────────────────────────────────────
void TracerPanel::drawTracks(QPainter &p, const QRect &area,
                             qint64 startMs, qint64 visibleMs)
{
    const qint64 endMs = startMs + visibleMs;

    if (m_tracks.isEmpty())
    {
        QFont f;
        f.setPointSize(11);
        p.setFont(f);
        p.setPen(QColor(0x4B, 0x55, 0x63));
        p.drawText(QRect(LaneHeaderW, area.y(), width() - LaneHeaderW, area.height()),
                   Qt::AlignCenter,
                   QStringLiteral("Waiting for RTOS events…\n"
                                  "Connect and start your target."));
        return;
    }

    // Blink phase for "currently running" bars
    const bool blink = (QDateTime::currentMSecsSinceEpoch() % 1000) < 500;

    for (int i = 0; i < m_tracks.size(); ++i)
    {
        const TaskTrack &track = m_tracks[i];
        const int laneTop = area.y() + i * LaneHeight - m_vScrollOffset;
        const int laneMid = laneTop + LaneHeight / 2;

        if (laneTop + LaneHeight < area.y())
            continue;

        if (laneTop > area.bottom())
            break;

        // ── Lane background ──────────────────────────────────────────────────
        const QColor laneBg = (i % 2 == 0) ? QColor(0x10, 0x17, 0x20)
                                           : QColor(0x0D, 0x13, 0x1A);
        p.fillRect(LaneHeaderW, laneTop, area.width() - LaneHeaderW, LaneHeight, laneBg);

        // ── Lane header (left column) ────────────────────────────────────────
        p.fillRect(0, laneTop, LaneHeaderW, LaneHeight, QColor(0x16, 0x1B, 0x22));

        // Colour strip
        p.setPen(Qt::NoPen);
        p.setBrush(track.color);
        p.drawRoundedRect(4, laneTop + 11, 4, LaneHeight - 22, 2, 2);

        // Task name
        QFont nf;
        nf.setPointSize(9);
        p.setFont(nf);
        p.setPen(QColor(0xE5, 0xE7, 0xEB));
        p.drawText(14, laneTop, LaneHeaderW - 30, LaneHeight,
                   Qt::AlignLeft | Qt::AlignVCenter, track.name);

        // ID tag (right-aligned in header)
        QFont idf;
        idf.setPointSize(7);
        p.setFont(idf);
        p.setPen(QColor(0x6B, 0x72, 0x80));
        p.drawText(14, laneTop, LaneHeaderW - 8, LaneHeight,
                   Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("ID:%1").arg(track.id));

        // Lane bottom separator
        p.setPen(QColor(0x22, 0x2C, 0x38));
        p.drawLine(0, laneTop + LaneHeight - 1, width(), laneTop + LaneHeight - 1);

        // ── Activity bars ────────────────────────────────────────────────────
        const int barH = LaneHeight - 22;
        const int barY = laneTop + 11;
        const int plotX = LaneHeaderW;
        const int plotW = area.width() - LaneHeaderW;

        for (const auto &range : track.ranges)
        {
            const double rStartMs = range.startUs / 1000.0;
            const double rEndMs = range.endUs / 1000.0;
            if (rEndMs < startMs || rStartMs > endMs)
                continue;

            const int x1 = plotX + static_cast<int>(qMax(0.0, rStartMs - startMs) * m_pixelsPerMs);
            const int x2 = plotX + static_cast<int>((qMin(rEndMs, static_cast<double>(endMs)) - startMs) * m_pixelsPerMs);
            const int w = qMax(1, x2 - x1);

            p.setPen(Qt::NoPen);
            p.setBrush(track.color.darker(200));
            p.drawRoundedRect(x1, barY, w, barH, 3, 3);
            p.setPen(QPen(track.color, 1));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(x1, barY, w, barH, 3, 3);
        }

        // Ongoing (still SWITCHED IN) bar — blinks
        if (track.activeStart >= 0)
        {
            const double aMs = track.activeStart / 1000.0;
            if (aMs <= endMs)
            {
                const int x1 = plotX + static_cast<int>(qMax(0.0, aMs - startMs) * m_pixelsPerMs);
                const int x2 = plotX + plotW;
                const QColor fill = blink ? track.color.darker(140) : track.color.darker(190);

                p.setPen(Qt::NoPen);
                p.setBrush(fill);
                p.drawRoundedRect(x1, barY, x2 - x1, barH, 3, 3);
                p.setPen(QPen(track.color.lighter(140), 1));
                p.setBrush(Qt::NoBrush);
                p.drawRoundedRect(x1, barY, x2 - x1, barH, 3, 3);
            } // closes if(aMs <= endMs)
        } // closes if(track.activeStart >= 0)

        // ── Event marker diamonds ─────────────────────────────────────────────
        constexpr int D = 6;
        for (const auto &marker : track.markers)
        {
            const double mMs = marker.timeUs / 1000.0;
            if (mMs < startMs || mMs > endMs)
                continue;
            const int mx = plotX + static_cast<int>((mMs - startMs) * m_pixelsPerMs);

            QColor mc;
            switch (marker.type)
            {
            case RtosEventType::Initialized:
                mc = {0x60, 0xA5, 0xFA};
                break;
            case RtosEventType::PushedReInit:
                mc = {0xC0, 0x84, 0xFC};
                break;
            case RtosEventType::Terminated:
                mc = {0xF8, 0x71, 0x71};
                break;
            case RtosEventType::Suspended:
                mc = {0xFC, 0xD3, 0x4D};
                break;
            case RtosEventType::StackOverflow:
                mc = {0xFF, 0x33, 0x33};
                break;
            default:
                mc = {0x9C, 0xA3, 0xAF};
                break;
            }

            QPolygon diamond;
            diamond << QPoint(mx, laneMid - D)
                    << QPoint(mx + D, laneMid)
                    << QPoint(mx, laneMid + D)
                    << QPoint(mx - D, laneMid);
            p.setPen(Qt::NoPen);
            p.setBrush(mc);
            p.drawPolygon(diamond);
            p.setPen(mc.lighter(160));
            p.setBrush(Qt::NoBrush);
            p.drawPolygon(diamond);
        }
    }
}

// ─── Tooltip helpers ──────────────────────────────────────────────────────────
static QString formatUs(qint64 us)
{
    const qint64 ms = us / 1000;
    const qint64 rem = us % 1000;
    if (ms >= 60000)
        return QStringLiteral("%1m %2s %3ms %4\u00b5s")
            .arg(ms / 60000)
            .arg((ms % 60000) / 1000)
            .arg(ms % 1000)
            .arg(rem);
    if (ms >= 1000)
        return QStringLiteral("%1s %2ms %3\u00b5s")
            .arg(ms / 1000)
            .arg(ms % 1000)
            .arg(rem);
    return QStringLiteral("%1ms %2\u00b5s").arg(ms).arg(rem);
}

// ─── Mouse move → QToolTip ───────────────────────────────────────────────────
void TracerPanel::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const int sbH = m_hScrollBar->sizeHint().height();
    const int fullH = height() - sbH - BotPad;
    const int tracksY = TopPad + LegendH + TimeAxisH;
    const int tracksH = fullH - tracksY;

    const int sbW = m_vScrollBar->isVisible() ? m_vScrollBar->width() : 0;
    if (pos.x() > width() - sbW)
    {
        QToolTip::hideText();
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (pos.x() < LaneHeaderW || pos.y() < tracksY || pos.y() > tracksY + tracksH)
    {
        QToolTip::hideText();
        QWidget::mouseMoveEvent(event);
        return;
    }

    // Cursor position in µs
    const double cursorMs = m_scrollOffsetMs + (pos.x() - LaneHeaderW) / m_pixelsPerMs;
    const qint64 cursorUs = static_cast<qint64>(cursorMs * 1000.0);

    const int row = (pos.y() - tracksY + m_vScrollOffset) / LaneHeight;
    if (row < 0 || row >= m_tracks.size())
    {
        QToolTip::hideText();
        QWidget::mouseMoveEvent(event);
        return;
    }

    const TaskTrack &track = m_tracks[row];
    const int laneMid = tracksY + row * LaneHeight - m_vScrollOffset + LaneHeight / 2;

    // ── Check event marker diamonds (hit radius = D + 4px) ───────────────────
    constexpr int D = 6;
    constexpr int hitR = D + 4;
    for (const auto &marker : track.markers)
    {
        const double mMs = marker.timeUs / 1000.0;
        const int mx = LaneHeaderW + static_cast<int>((mMs - m_scrollOffsetMs) * m_pixelsPerMs);
        if (qAbs(pos.x() - mx) <= hitR && qAbs(pos.y() - laneMid) <= hitR)
        {
            const QString tip = QStringLiteral(
                                    "<b>%1</b> &nbsp;(ID: %2)<hr>"
                                    "Event: <b>%3</b><br>"
                                    "Time:&nbsp; %4")
                                    .arg(track.name)
                                    .arg(track.id)
                                    .arg(rtosEventName(marker.type))
                                    .arg(formatUs(marker.timeUs));
            QToolTip::showText(QCursor::pos(), tip, this);
            return;
        }
    }

    // ── Check activity bars ───────────────────────────────────────────────────
    const int barY = tracksY + row * LaneHeight - m_vScrollOffset + 11;
    const int barH = LaneHeight - 22;
    if (pos.y() >= barY && pos.y() <= barY + barH)
    {
        for (const auto &range : track.ranges)
        {
            if (cursorUs >= range.startUs && cursorUs <= range.endUs)
            {
                const qint64 durUs = range.endUs - range.startUs;
                const QString tip = QStringLiteral(
                                        "<b>%1</b> &nbsp;(ID: %2)<hr>"
                                        "Start:&nbsp;&nbsp;&nbsp;&nbsp; %3<br>"
                                        "End:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; %4<br>"
                                        "Duration: %5")
                                        .arg(track.name)
                                        .arg(track.id)
                                        .arg(formatUs(range.startUs))
                                        .arg(formatUs(range.endUs))
                                        .arg(formatUs(durUs));
                QToolTip::showText(QCursor::pos(), tip, this);
                return;
            }
        }
        // Ongoing (still running) bar
        if (track.activeStart >= 0 && cursorUs >= track.activeStart)
        {
            const QString tip = QStringLiteral(
                                    "<b>%1</b> &nbsp;(ID: %2)<hr>"
                                    "Start:&nbsp;&nbsp;&nbsp;&nbsp; %3<br>"
                                    "Status: <b>RUNNING</b>")
                                    .arg(track.name)
                                    .arg(track.id)
                                    .arg(formatUs(track.activeStart));
            QToolTip::showText(QCursor::pos(), tip, this);
            return;
        }
    }

    // ── Fallback: show cursor time ────────────────────────────────────────────
    QToolTip::showText(QCursor::pos(),
                       QStringLiteral("%1\nTime: %2").arg(track.name, formatUs(cursorUs)), this);
    QWidget::mouseMoveEvent(event);
}

void TracerPanel::leaveEvent(QEvent *event)
{
    QToolTip::hideText();
    QWidget::leaveEvent(event);
}
