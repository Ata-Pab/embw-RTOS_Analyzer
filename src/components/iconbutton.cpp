#include "iconbutton.h"

#include "theme/theme.h"

#include <QEnterEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

using namespace Theme;

// ── Geometry constants ────────────────────────────────────────────────────────
static constexpr int IconTextGap = 6; ///< Pixels between icon and label text

// ── ctor ──────────────────────────────────────────────────────────────────────
IconButton::IconButton(const QString &svgPath,
                       const QString &text,
                       Variant variant,
                       QWidget *parent,
                       bool showFocusRing)
    : QPushButton(text, parent), m_svgPath(svgPath), m_renderer(new QSvgRenderer(this)), m_variant(variant), m_colorAnim(new QVariantAnimation(this)), m_showFocusRing(showFocusRing)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setAttribute(Qt::WA_Hover);

    // Load SVG (parser runs once here; pixel cache built lazily in paintEvent)
    if (!svgPath.isEmpty())
        m_renderer->load(svgPath);

    m_colorAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_colorAnim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &val)
            { setCurrentColor(val.value<QColor>()); });

    updatePalette();
}

// ── Public API ────────────────────────────────────────────────────────────────
void IconButton::setSvgPath(const QString &path)
{
    if (m_svgPath == path)
        return;

    m_svgPath = path;
    m_renderer->load(path);
    invalidateIconCache();
    update();
}

void IconButton::setIconDisplaySize(const QSize &size)
{
    if (m_iconSize == size)
        return;

    m_iconSize = size;
    invalidateIconCache();
    updateGeometry();
    update();
}

void IconButton::setColorizeIcon(bool on)
{
    if (m_colorizeIcon == on)
        return;

    m_colorizeIcon = on;
    update(); // no cache invalidation needed; compositing happens in paintEvent
}

void IconButton::setVariant(Variant variant)
{
    m_variant = variant;
    updatePalette();
    update();
}

void IconButton::setCurrentColor(const QColor &color)
{
    m_currentColor = color;
    update();
}

QSize IconButton::sizeHint() const
{
    const int iconW = m_iconSize.width();
    const int iconH = m_iconSize.height();
    const int paddingH = Spacing::ButtonPaddingH;
    const int paddingV = Spacing::ButtonPaddingV;

    int totalW = paddingH * 2 + iconW;
    int totalH = paddingV * 2 + iconH;

    if (!text().isEmpty())
    {
        const QFontMetrics fm(font());
        totalW += IconTextGap + fm.horizontalAdvance(text());
        totalH = qMax(totalH, fm.height() + paddingV * 2);
    }

    return {totalW, totalH};
}

// ── Icon cache helpers ────────────────────────────────────────────────────────

void IconButton::invalidateIconCache()
{
    m_cacheDirty = true;
    m_cachedDpr = 0.0;
}

void IconButton::ensureIconCache()
{
    const qreal dpr = devicePixelRatioF();

    // Rebuild only when dirty or the screen's DPR has changed (e.g. moved to
    // another monitor with different scaling).
    if (!m_cacheDirty && qFuzzyCompare(m_cachedDpr, dpr))
        return;

    if (!m_renderer->isValid() || m_svgPath.isEmpty())
    {
        m_iconCache = QPixmap{};
        m_cacheDirty = false;
        m_cachedDpr = dpr;
        return;
    }

    // Allocate at physical-pixel resolution so the icon is crisp on HiDPI.
    const QSize physSize(qRound(m_iconSize.width() * dpr),
                         qRound(m_iconSize.height() * dpr));

    QPixmap px(physSize);
    px.fill(Qt::transparent);

    {
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        // Render the SVG at the full physical-pixel rectangle.
        m_renderer->render(&p, QRectF(QPointF(0, 0), QSizeF(physSize)));
    }

    // Tag the pixmap with the DPR so Qt scales it correctly when painted.
    px.setDevicePixelRatio(dpr);

    m_iconCache = px;
    m_cacheDirty = false;
    m_cachedDpr = dpr;
}

// ── Private helpers ───────────────────────────────────────────────────────────
void IconButton::updatePalette()
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
        m_textColor = Color::Blue500;
        m_borderColor = Color::Gray200;
        break;
    }

    m_currentColor = isEnabled() ? m_baseColor : Color::Gray200;
}

void IconButton::animateTo(const QColor &target, int durationMs)
{
    m_colorAnim->stop();
    m_colorAnim->setDuration(durationMs);
    m_colorAnim->setStartValue(m_currentColor);
    m_colorAnim->setEndValue(target);
    m_colorAnim->start();
}

// ── Painting ──────────────────────────────────────────────────────────────────
void IconButton::paintEvent(QPaintEvent *)
{
    ensureIconCache(); // no-op when cache is valid

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);

    // ── Background ────────────────────────────────────────────────────────────
    QPainterPath bgPath;
    bgPath.addRoundedRect(r, Spacing::RadiusMd, Spacing::RadiusMd);

    const QColor bg = isEnabled() ? m_currentColor : Color::Gray200;
    p.fillPath(bgPath, bg);

    // ── Ghost border ──────────────────────────────────────────────────────────
    if (m_variant == Variant::Ghost && isEnabled())
    {
        p.setPen(QPen(m_borderColor, Spacing::BorderDefault));
        p.drawPath(bgPath);
    }

    // ── Focus ring ────────────────────────────────────────────────────────────
    if (m_showFocusRing && hasFocus() && isEnabled())
    {
        QPainterPath focusPath;
        focusPath.addRoundedRect(QRectF(rect()).adjusted(2, 2, -2, -2),
                                 Spacing::RadiusMd + 1, Spacing::RadiusMd + 1);
        p.setPen(QPen(Color::Blue300, Spacing::BorderThick));
        p.drawPath(focusPath);
    }

    // ── Icon + text layout ────────────────────────────────────────────────────
    const QColor iconColor = isEnabled() ? m_textColor : Color::Gray400;
    const bool hasText = !text().isEmpty();
    const bool hasIcon = !m_iconCache.isNull();

    const int paddingH = Spacing::ButtonPaddingH;
    const int iconW = m_iconSize.width();
    const int iconH = m_iconSize.height();

    if (hasIcon && !hasText)
    {
        // Centred icon-only layout
        const QRect iconRect(
            (width() - iconW) / 2,
            (height() - iconH) / 2,
            iconW, iconH);

        if (m_colorizeIcon)
        {
            // Use the pre-rendered pixmap as an alpha mask and fill with
            // the theme colour. CompositionMode_SourceIn retains only the
            // alpha of the destination, multiplied by the source alpha.
            QPixmap tinted(m_iconCache.size());
            tinted.setDevicePixelRatio(m_iconCache.devicePixelRatio());
            tinted.fill(Qt::transparent);

            QPainter tp(&tinted);
            tp.drawPixmap(0, 0, m_iconCache);
            tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
            tp.fillRect(tinted.rect(), iconColor);

            p.drawPixmap(iconRect, tinted);
        }
        else
        {
            p.drawPixmap(iconRect, m_iconCache);
        }
    }
    else if (hasIcon && hasText)
    {
        // Icon-left + text layout
        const QFontMetrics fm(font());
        const int textW = fm.horizontalAdvance(text());
        const int totalW = paddingH + iconW + IconTextGap + textW + paddingH;

        int x = (width() - totalW) / 2 + paddingH;

        const QRect iconRect(x, (height() - iconH) / 2, iconW, iconH);

        if (m_colorizeIcon)
        {
            QPixmap tinted(m_iconCache.size());
            tinted.setDevicePixelRatio(m_iconCache.devicePixelRatio());
            tinted.fill(Qt::transparent);

            QPainter tp(&tinted);
            tp.drawPixmap(0, 0, m_iconCache);
            tp.setCompositionMode(QPainter::CompositionMode_SourceIn);
            tp.fillRect(tinted.rect(), iconColor);

            p.drawPixmap(iconRect, tinted);
        }
        else
        {
            p.drawPixmap(iconRect, m_iconCache);
        }

        x += iconW + IconTextGap;

        p.setPen(iconColor);
        p.setFont(font());
        p.drawText(QRect(x, 0, textW, height()), Qt::AlignVCenter | Qt::AlignLeft, text());
    }
    else if (hasText)
    {
        // Text-only fallback (no SVG loaded)
        p.setPen(iconColor);
        p.setFont(font());
        p.drawText(rect(), Qt::AlignCenter, text());
    }
}

// ── Events ────────────────────────────────────────────────────────────────────
void IconButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    // DPR might change when the window is dragged to a different monitor;
    // invalidate if so (actual check happens lazily in ensureIconCache).
    if (!qFuzzyCompare(devicePixelRatioF(), m_cachedDpr))
        invalidateIconCache();
}

void IconButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    if (!isEnabled())
        return;
    m_hovered = true;
    animateTo(m_hoverColor, Animation::Default);
}

void IconButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    if (!isEnabled())
        return;
    m_hovered = false;
    m_pressed = false;
    animateTo(m_baseColor, Animation::Slow);
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    if (!isEnabled())
        return;
    m_pressed = true;
    animateTo(m_pressColor, Animation::Fast);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    if (!isEnabled())
        return;
    m_pressed = false;
    animateTo(m_hovered ? m_hoverColor : m_baseColor, Animation::Default);
}

void IconButton::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
    {
        updatePalette();
        update();
    }
}
