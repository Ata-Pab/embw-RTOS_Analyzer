#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "network/udpreceiver.h"
#include "panels/logpanel.h"
#include "panels/tracerpanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Toolbar style (GitHub-dark-inspired)
// ---------------------------------------------------------------------------
static const char *kToolbarStyle =
    "QWidget#toolbar {"
    "  background: #161B22;"
    "  border-bottom: 1px solid #30405060;"
    "}"
    "QLabel { background: transparent; border: none; color: #8B949E; }"
    "QLineEdit, QSpinBox {"
    "  background: #0D1117; color: #C9D1D9;"
    "  border: 1px solid #30363D; border-radius: 4px; padding: 2px 6px;"
    "}"
    "QPushButton {"
    "  background: #21262D; color: #C9D1D9;"
    "  border: 1px solid #30363D; border-radius: 4px; padding: 4px 14px;"
    "}"
    "QPushButton:hover   { background: #30363D; }"
    "QPushButton:pressed { background: #161B22; }"
    "QPushButton:disabled { color: #718397; background: #161B22; }";

// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Message Tracer"));
    resize(1280, 820);
    setStyleSheet(QStringLiteral("QMainWindow { background: #0D1117; }"));

    m_receiver = new UdpReceiver(this);
    connect(m_receiver, &UdpReceiver::eventReceived, this, &MainWindow::onEventReceived);
    connect(m_receiver, &UdpReceiver::errorOccurred, this, &MainWindow::onError);

    buildUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------------
void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    central->setStyleSheet(QStringLiteral("background: #0D1117;"));
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Toolbar ───────────────────────────────────────────────────────────────
    auto *toolbar = new QWidget(central);
    toolbar->setObjectName(QStringLiteral("toolbar"));
    toolbar->setFixedHeight(52);
    toolbar->setStyleSheet(QString::fromLatin1(kToolbarStyle));

    auto *tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(12, 6, 12, 6);
    tl->setSpacing(8);

    // Title
    auto *appTitle = new QLabel(QStringLiteral("Port Messages"), toolbar);
    QFont tf;
    tf.setPointSize(12);
    tf.setBold(true);
    appTitle->setFont(tf);
    appTitle->setFixedWidth(154);
    appTitle->setStyleSheet(QStringLiteral("color: #58A6FF;"));

    // Vertical separator helper
    auto mkSep = [toolbar]()
    {
        auto *sep = new QFrame(toolbar);
        sep->setFrameShape(QFrame::VLine);
        sep->setStyleSheet(QStringLiteral("color: #718397;"));
        sep->setFixedWidth(1);
        return sep;
    };

    // IP / Port
    auto *ipLabel = new QLabel(QStringLiteral("IP:"), toolbar);
    m_ipEdit = new QLineEdit(QStringLiteral("127.0.0.1"), toolbar);
    m_ipEdit->setFixedWidth(112);
    m_ipEdit->setToolTip(QStringLiteral("UDP listen address"));

    auto *portLabel = new QLabel(QStringLiteral("Port:"), toolbar);
    m_portSpin = new QSpinBox(toolbar);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(9999);
    m_portSpin->setFixedWidth(72);
    m_portSpin->setToolTip(QStringLiteral("UDP port (1–65535)"));

    // Connect / Disconnect
    m_connectBtn = new QPushButton(QStringLiteral("Connect"), toolbar);
    m_connectBtn->setStyleSheet(QStringLiteral(
        "QPushButton         { background:#238636; color:#FFF; border:1px solid #2EA043; border-radius:4px; padding:4px 14px; }"
        "QPushButton:hover   { background:#2EA043; }"
        "QPushButton:pressed { background:#1A7F37; }"));

    m_disconnectBtn = new QPushButton(QStringLiteral("Disconnect"), toolbar);
    m_disconnectBtn->setEnabled(false);
    m_disconnectBtn->setStyleSheet(QStringLiteral(
        "QPushButton         { background:#B91C1C; color:#FFF; border:1px solid #EF4444; border-radius:4px; padding:4px 14px; }"
        "QPushButton:hover   { background:#DC2626; }"
        "QPushButton:pressed { background:#991B1B; }"
        "QPushButton:disabled { background:#21262D; color:#718397; border-color:#30363D; }"));

    auto *clearBtn = new QPushButton(QStringLiteral("Clear"), toolbar);

    // Status indicators (right side)
    m_statusDot = new QLabel(QStringLiteral("\u25CF"), toolbar);
    m_statusDot->setStyleSheet(QStringLiteral("color:#718397; font-size:14px;"));

    m_statusLabel = new QLabel(QStringLiteral("Disconnected"), toolbar);

    m_eventCountLabel = new QLabel(QStringLiteral("Events: 0"), toolbar);

    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    tl->addWidget(appTitle);
    tl->addWidget(mkSep());
    tl->addWidget(ipLabel);
    tl->addWidget(m_ipEdit);
    tl->addWidget(portLabel);
    tl->addWidget(m_portSpin);
    tl->addWidget(m_connectBtn);
    tl->addWidget(m_disconnectBtn);
    tl->addWidget(mkSep());
    tl->addWidget(clearBtn);
    tl->addStretch();
    tl->addWidget(m_statusDot);
    tl->addWidget(m_statusLabel);
    tl->addWidget(mkSep());
    tl->addWidget(m_eventCountLabel);

    // ── Splitter: TracerPanel (top) + LogPanel (bottom) ───────────────────────
    auto *splitter = new QSplitter(Qt::Vertical, central);
    splitter->setStyleSheet(QStringLiteral(
        "QSplitter::handle { background: #30363D; height: 3px; }"));

    // Tracer inside a QScrollArea for vertical overflow when many tasks are shown
    m_tracerPanel = new TracerPanel;

    auto *tracerScroll = new QScrollArea;
    tracerScroll->setWidget(m_tracerPanel);
    tracerScroll->setWidgetResizable(true);
    tracerScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tracerScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tracerScroll->setStyleSheet(QStringLiteral(
        "QScrollArea { border: none; background: #0D1117; }"
        "QScrollBar:vertical { background:#161B22; width:8px; border-radius:4px; }"
        "QScrollBar::handle:vertical { background:#30363D; border-radius:4px; min-height:20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"));

    m_logPanel = new LogPanel;

    splitter->addWidget(tracerScroll);
    splitter->addWidget(m_logPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({520, 280});

    root->addWidget(toolbar);
    root->addWidget(splitter, 1);

    setCentralWidget(central);

    statusBar()->setStyleSheet(QStringLiteral("background:#161B22; color:#8B949E;"));
    statusBar()->showMessage(
        QStringLiteral("Ready — set IP / Port and click Connect to start listening."));
}

// ─── Slots ────────────────────────────────────────────────────────────────────
void MainWindow::onConnectClicked()
{
    const QString ip = m_ipEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_portSpin->value());
    const QHostAddress addr(ip.isEmpty() ? QStringLiteral("127.0.0.1") : ip);

    if (m_receiver->start(addr, port))
    {
        setConnected(true);
        statusBar()->showMessage(
            QStringLiteral("Listening on %1:%2 \u2026").arg(ip).arg(port));
    }
}

void MainWindow::onDisconnectClicked()
{
    m_receiver->stop();
    setConnected(false);
    statusBar()->showMessage(QStringLiteral("Disconnected."));
}

void MainWindow::onClearClicked()
{
    m_logPanel->clear();
    m_tracerPanel->clear();
    m_eventCount = 0;
    m_eventCountLabel->setText(QStringLiteral("Events: 0"));
}

void MainWindow::onEventReceived(const RtosEvent &event)
{
    m_logPanel->appendEvent(event);
    m_tracerPanel->appendEvent(event);
    m_eventCountLabel->setText(
        QStringLiteral("Events: %1").arg(++m_eventCount));
}

void MainWindow::onError(const QString &msg)
{
    setConnected(false);
    statusBar()->showMessage(QStringLiteral("Error: %1").arg(msg));
}

void MainWindow::setConnected(bool connected)
{
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    m_ipEdit->setEnabled(!connected);
    m_portSpin->setEnabled(!connected);

    if (connected)
    {
        m_statusDot->setStyleSheet(QStringLiteral("color:#3FB950; font-size:14px;"));
        m_statusLabel->setText(QStringLiteral("Connected"));
        m_statusLabel->setStyleSheet(QStringLiteral("color:#3FB950;"));
    }
    else
    {
        m_statusDot->setStyleSheet(QStringLiteral("color:#718397; font-size:14px;"));
        m_statusLabel->setText(QStringLiteral("Disconnected"));
        m_statusLabel->setStyleSheet(QStringLiteral("color:#8B949E;"));
    }
}
