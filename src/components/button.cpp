#include "button.h"

#include "theme/theme.h"

#include <QEnterEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

using namespace Theme;

// ── ctor ─────────────────────────────────────────────────────────────────────
Button::Button(const QString &text, Variant variant, QWidget *parent, bool showFocusRing)
    : QPushButton(text, parent), m_variant(variant), m_colorAnim(new QVariantAnimation(this)), m_showFocusRing(showFocusRing)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true); // suppress platform default painting
    setAttribute(Qt::WA_Hover);

    m_colorAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_colorAnim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &val)
            { setCurrentColor(val.value<QColor>()); });

    updatePalette();
}

// ── public API ────────────────────────────────────────────────────────────────
void Button::setVariant(Variant variant)
{
    m_variant = variant;
    updatePalette();
    update();
}

void Button::setCurrentColor(const QColor &color)
{
    m_currentColor = color;
    update();
}

QSize Button::sizeHint() const
{
    const QFontMetrics fm(font());
    return {fm.horizontalAdvance(text()) + Spacing::ButtonPaddingH * 2,
            fm.height() + Spacing::ButtonPaddingV * 2};
}

// ── private helpers ───────────────────────────────────────────────────────────
void Button::updatePalette()
{
    switch (m_variant)
    {
    case Variant::Primary:
        m_baseColor = Color::Blue500;
        m_hoverColor = Color::Blue600;
        m_pressColor = Color::Blue700;
        m_textColor = Color::White;
        m_borderColor = Qt::transparent;
        break;
    case Variant::Secondary:
        m_baseColor = Color::Slate500;
        m_hoverColor = Color::Slate600;
        m_pressColor = Color::Slate700;
        m_textColor = Color::White;
        m_borderColor = Qt::transparent;
        break;
    case Variant::Danger:
        m_baseColor = Color::Red500;
        m_hoverColor = Color::Red600;
        m_pressColor = Color::Red700;
        m_textColor = Color::White;
        m_borderColor = Qt::transparent;
        break;
    case Variant::Ghost:
        m_baseColor = Qt::transparent;
        m_hoverColor = QColor(Color::Blue500.red(), Color::Blue500.green(),
                              Color::Blue500.blue(), 25); // ~10 % opacity
        m_pressColor = QColor(Color::Blue500.red(), Color::Blue500.green(),
                              Color::Blue500.blue(), 50); // ~20 % opacity
        m_textColor = Color::Gray700;
        m_borderColor = Color::Gray400;
        // m_borderColor = Color::Blue500;
        // m_textColor = Color::Blue500;
        break;
    }

    m_currentColor = isEnabled() ? m_baseColor : Color::Gray200;
}

void Button::animateTo(const QColor &target, int durationMs)
{
    m_colorAnim->stop();
    m_colorAnim->setDuration(durationMs);
    m_colorAnim->setStartValue(m_currentColor);
    m_colorAnim->setEndValue(target);
    m_colorAnim->start();
}

// ── painting ──────────────────────────────────────────────────────────────────
void Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);

    // Background fill
    QPainterPath bgPath;
    bgPath.addRoundedRect(r, Spacing::RadiusMd, Spacing::RadiusMd);
    const QColor bg = isEnabled() ? m_currentColor : Color::Gray200;
    p.fillPath(bgPath, bg);

    // Ghost border
    if (m_variant == Variant::Ghost && isEnabled())
    {
        p.setPen(QPen(m_borderColor, Spacing::BorderDefault));
        p.drawPath(bgPath);
    }

    // Keyboard-focus ring
    if ((m_showFocusRing) && (hasFocus()) && (isEnabled()))
    {
        QPen focusPen(Color::Blue300, Spacing::BorderThick);
        p.setPen(focusPen);
        QPainterPath focusPath;
        focusPath.addRoundedRect(QRectF(rect()).adjusted(2, 2, -2, -2),
                                 Spacing::RadiusMd + 1, Spacing::RadiusMd + 1);
        p.drawPath(focusPath);
    }

    // Label text
    p.setPen(isEnabled() ? m_textColor : Color::Gray400);
    p.setFont(font());
    p.drawText(rect(), Qt::AlignCenter, text());
}

// ── events ────────────────────────────────────────────────────────────────────
void Button::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    if (!isEnabled())
        return;
    m_hovered = true;
    animateTo(m_hoverColor, Animation::Default);
}

void Button::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    if (!isEnabled())
        return;
    m_hovered = false;
    m_pressed = false;
    animateTo(m_baseColor, Animation::Slow);
}

void Button::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    if (!isEnabled())
        return;
    m_pressed = true;
    animateTo(m_pressColor, Animation::Fast);
}

void Button::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    if (!isEnabled())
        return;
    m_pressed = false;
    animateTo(m_hovered ? m_hoverColor : m_baseColor, Animation::Default);
}

void Button::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
    {
        updatePalette();
        update();
    }
}
