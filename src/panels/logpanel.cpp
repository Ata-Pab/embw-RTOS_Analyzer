#include "logpanel.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QScrollBar>

static const char *kLogPanelStyle =
    "QPlainTextEdit {"
    "  background-color: #0D1117;"
    "  color: #B0C4DE;"
    "  border: 1px solid #30405060;"
    "  border-radius: 4px;"
    "  selection-background-color: #1F6FEB40;"
    "}"
    "QScrollBar:vertical {"
    "  background: #161B22; width: 8px; border-radius: 4px;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: #30363D; border-radius: 4px; min-height: 20px;"
    "}"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }";

LogPanel::LogPanel(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(120);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);

    // ── Header bar ────────────────────────────────────────────────────────────
    auto *header = new QHBoxLayout;
    header->setContentsMargins(0, 0, 0, 0);

    auto *title = new QLabel(QStringLiteral("Raw Log"), this);
    QFont tf;
    tf.setPointSize(10);
    tf.setBold(true);
    title->setFont(tf);
    title->setStyleSheet(QStringLiteral("color: #8B949E;"));

    auto *autoScrollCb = new QCheckBox(QStringLiteral("Auto-scroll"), this);
    autoScrollCb->setChecked(true);
    autoScrollCb->setStyleSheet(QStringLiteral("color: #8B949E;"));
    connect(autoScrollCb, &QCheckBox::toggled, this, [this](bool on)
            { m_autoScroll = on; });

    header->addWidget(title);
    header->addStretch();
    header->addWidget(autoScrollCb);

    // ── Text area ─────────────────────────────────────────────────────────────
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont(QStringLiteral("Courier New"), 9));
    m_textEdit->setMaximumBlockCount(5000);
    m_textEdit->setStyleSheet(QString::fromLatin1(kLogPanelStyle));
    m_textEdit->setPlaceholderText(
        QStringLiteral("Waiting for UDP events — configure the address above and click Connect."));

    layout->addLayout(header);
    layout->addWidget(m_textEdit, 1);
}

void LogPanel::appendEvent(const RtosEvent &event)
{
    // Pick a color per event type
    const char *color;
    switch (event.eventType)
    {
    case RtosEventType::SwitchedIn:
        color = "#4ADE80";
        break; // green
    case RtosEventType::SwitchedOut:
        color = "#F97316";
        break; // orange
    case RtosEventType::Initialized:
        color = "#60A5FA";
        break; // blue
    case RtosEventType::PushedReInit:
        color = "#C084FC";
        break; // purple
    case RtosEventType::Terminated:
        color = "#F87171";
        break; // red
    case RtosEventType::Suspended:
        color = "#FCD34D";
        break; // yellow
    case RtosEventType::StackOverflow:
        color = "#FF3333";
        break; // bright red
    default:
        color = "#9CA3AF";
        break; // gray
    }

    m_textEdit->appendHtml(
        QStringLiteral(
            "<span style='color:%1; font-family:\"Courier New\"; font-size:9pt;'>%2</span>")
            .arg(QLatin1String(color), event.rawLine.toHtmlEscaped()));

    if (m_autoScroll)
        m_textEdit->verticalScrollBar()->setValue(
            m_textEdit->verticalScrollBar()->maximum());
}

void LogPanel::clear()
{
    m_textEdit->clear();
}
