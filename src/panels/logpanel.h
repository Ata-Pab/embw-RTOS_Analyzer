#pragma once

#include <QWidget>
#include "rtos/rtosevent.h"

class QPlainTextEdit;

/**
 * @class LogPanel
 * @brief Scrollable, color-coded raw UDP log view.
 *
 * Displays one line per RtosEvent in the format:
 *   HH:mm:ss.zzz: [Remote Log] (ID:N) taskName -> EVENT
 * Each event type gets a distinct foreground color.
 */
class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent = nullptr);

public slots:
    void appendEvent(const RtosEvent &event);
    void clear();

private:
    QPlainTextEdit *m_textEdit{nullptr};
    bool            m_autoScroll{true};
};
