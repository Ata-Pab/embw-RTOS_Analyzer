#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "components/button.h"
#include "theme/theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "components/iconbutton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    /* UI Design FILE */
    ui->setupUi(this);
    setWindowTitle("Modern Button Demo");
    resize(700, 420);

    // ── root container ────────────────────────────────────────────────────
    auto *container = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(container);
    rootLayout->setContentsMargins(Theme::Spacing::Xxl, Theme::Spacing::Xxl,
                                   Theme::Spacing::Xxl, Theme::Spacing::Xxl);
    rootLayout->setSpacing(Theme::Spacing::Xl + 4);
    container->setStyleSheet(
        QStringLiteral("background-color: %1;").arg(Theme::Color::White.name()));

    // ── title ─────────────────────────────────────────────────────────────
    auto *title = new QLabel("Modern Button Component", container);
    title->setFont(Theme::Typography::heading());
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        QStringLiteral("color: %1;").arg(Theme::Color::Gray900.name()));

    // ── subtitle ──────────────────────────────────────────────────────────
    auto *subtitle = new QLabel(
        "Animated variants \u2014 Primary \u00B7 Secondary \u00B7 Danger \u00B7 Ghost \u00B7 Disabled",
        container);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(
        QStringLiteral("color: %1; font-size: %2pt;")
            .arg(Theme::Color::Gray500.name())
            .arg(Theme::Typography::SizeLg));

    // ── button row ────────────────────────────────────────────────────────
    auto *row = new QHBoxLayout;
    row->setSpacing(16);

    auto *btnPrimary = new Button("Primary", Button::Variant::Primary, container);
    auto *btnSecondary = new Button("Secondary", Button::Variant::Secondary, container);
    auto *btnDanger = new Button("Danger", Button::Variant::Danger, container);
    auto *btnGhost = new Button("Ghost", Button::Variant::Ghost, container);
    auto *btnDisabled = new Button("Disabled", Button::Variant::Primary, container);
    auto *btnIcon = new IconButton("../resources/icons/apple_icon.svg", "", IconButton::Variant::Ghost, container);
    btnDisabled->setEnabled(false);

    for (auto *btn : {btnPrimary, btnSecondary, btnDanger, btnGhost, btnDisabled})
    {
        btn->setMinimumHeight(Theme::Spacing::ButtonMinHeight);
        btn->setMinimumWidth(Theme::Spacing::ButtonMinWidth);
        QFont f = btn->font();
        f.setPointSize(Theme::Typography::SizeMd);
        btn->setFont(f);
        row->addWidget(btn);
    }
    btnIcon->setMinimumHeight(Theme::Spacing::ButtonMinHeight);
    btnIcon->setMinimumWidth(Theme::Spacing::ButtonMinWidth);
    QFont f = btnIcon->font();
    f.setPointSize(Theme::Typography::SizeMd);
    btnIcon->setFont(f);
    row->addWidget(btnIcon);

    // ── status label ──────────────────────────────────────────────────────
    m_statusLabel = new QLabel("Hover or click a button.", container);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        QStringLiteral("color: %1; background: %2; border-radius: %3px; padding: %4px %5px;")
            .arg(Theme::Color::Gray700.name())
            .arg(Theme::Color::Gray100.name())
            .arg(Theme::Spacing::RadiusSm + 2)
            .arg(Theme::Spacing::Sm)
            .arg(Theme::Spacing::Lg));

    // ── connections ───────────────────────────────────────────────────────
    connect(btnPrimary, &QPushButton::clicked, this,
            [this]
            { m_statusLabel->setText("\u2713 Primary button clicked!"); });
    connect(btnSecondary, &QPushButton::clicked, this,
            [this]
            { m_statusLabel->setText("\u2713 Secondary button clicked!"); });
    connect(btnDanger, &QPushButton::clicked, this,
            [this]
            { m_statusLabel->setText("\u26A0 Danger button clicked!"); });
    connect(btnGhost, &QPushButton::clicked, this,
            [this]
            { m_statusLabel->setText("\u25CB Ghost button clicked!"); });
    connect(btnIcon, &QPushButton::clicked, this,
            [this]
            { m_statusLabel->setText("\u2714 Icon button clicked!"); });

    // ── assemble ──────────────────────────────────────────────────────────
    rootLayout->addStretch(1);
    rootLayout->addWidget(title);
    rootLayout->addWidget(subtitle);
    rootLayout->addLayout(row);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addStretch(1);

    setCentralWidget(container);
}

MainWindow::~MainWindow()
{
    delete ui;
}
