#pragma once

#include <QColor>
#include <QPixmap>
#include <QPushButton>
#include <QSvgRenderer>
#include <QVariantAnimation>

/**
 * @class IconButton
 * @brief A push-button that renders an SVG icon with optional label text.
 *
 * Performance model
 * -----------------
 * The SVG is parsed once by QSvgRenderer and rasterized into a QPixmap cache
 * at the correct physical-pixel resolution (device-pixel-ratio aware). Every
 * subsequent paintEvent is a plain pixmap blit — no SVG re-parsing or
 * re-rasterization per frame.
 *
 * The cache is rebuilt only when:
 *   • the SVG path changes (setSvgPath)
 *   • the icon display size changes (setIconSize)
 *   • the widget's device-pixel-ratio changes (resizeEvent / changeEvent)
 *
 * Usage
 * -----
 *   // Icon-only button
 *   auto *btn = new IconButton(":/icons/save.svg", {}, IconButton::Variant::Primary, parent);
 *
 *   // Icon + label button
 *   auto *btn = new IconButton(":/icons/save.svg", "Save", IconButton::Variant::Ghost, parent);
 *
 * SVG colour notes
 * ----------------
 * The SVG is rendered with its natural colours. For monochrome icons whose
 * fill should follow the button accent colour, design the SVG with a solid
 * opaque fill and set m_colorizeIcon = true (see setColorizeIcon()).
 */
class IconButton : public QPushButton
{
    Q_OBJECT

    /// Exposed so QVariantAnimation can tween the background colour.
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor)

public:
    enum class Variant
    {
        Primary,
        Secondary,
        Danger,
        Ghost
    };
    Q_ENUM(Variant)

    explicit IconButton(const QString &svgPath,
                        const QString &text = {},
                        Variant variant = Variant::Primary,
                        QWidget *parent = nullptr,
                        bool showFocusRing = false);

    // ── SVG / icon ────────────────────────────────────────────────────────────
    void setSvgPath(const QString &path);
    QString svgPath() const { return m_svgPath; }

    void setIconDisplaySize(const QSize &size);
    QSize iconDisplaySize() const { return m_iconSize; }

    /**
     * When true (default: false) the icon is treated as an alpha mask and
     * filled with the button's text/icon colour. Useful for single-colour
     * SVG icons that should inherit the button's theme colour.
     */
    void setColorizeIcon(bool on);
    bool colorizeIcon() const { return m_colorizeIcon; }

    // ── Style variant ─────────────────────────────────────────────────────────
    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    // ── Geometry ──────────────────────────────────────────────────────────────
    QSize sizeHint() const override;

    // ── Colour property (animated) ────────────────────────────────────────────
    QColor currentColor() const { return m_currentColor; }
    void setCurrentColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void updatePalette();
    void animateTo(const QColor &target, int durationMs = 150);

    /// Re-rasterizes the SVG into m_iconCache at device-pixel-ratio resolution.
    /// No-op when the cache is already valid.
    void ensureIconCache();
    void invalidateIconCache();

    // ── Icon state ────────────────────────────────────────────────────────────
    QString m_svgPath;
    QSvgRenderer *m_renderer;
    QPixmap m_iconCache;
    QSize m_iconSize{20, 20};
    qreal m_cachedDpr{0.0}; ///< DPR used when cache was built
    bool m_cacheDirty{true};
    bool m_colorizeIcon{false};

    // ── Colour / style ────────────────────────────────────────────────────────
    Variant m_variant;

    QColor m_baseColor;
    QColor m_hoverColor;
    QColor m_pressColor;
    QColor m_textColor;
    QColor m_borderColor;
    QColor m_currentColor;

    bool m_hovered{false};
    bool m_pressed{false};
    bool m_showFocusRing{false};

    QVariantAnimation *m_colorAnim;
};
