#pragma once

#include <QColor>
#include <QPushButton>
#include <QVariantAnimation>

/**
 * @class Button
 * @brief A reusable, animated push-button with four style variants.
 *
 * Usage:
 *   auto *btn = new Button("Save", Button::Variant::Primary, parent);
 *   connect(btn, &QPushButton::clicked, this, &MyClass::onSave);
 */
class Button : public QPushButton
{
    Q_OBJECT

    /// Exposed so QPropertyAnimation / QVariantAnimation can tween it.
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

    explicit Button(const QString &text,
                    Variant variant = Variant::Primary,
                    QWidget *parent = nullptr,
                    bool showFocusRing = false);

    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    QColor currentColor() const { return m_currentColor; }
    void setCurrentColor(const QColor &color);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void updatePalette();
    void animateTo(const QColor &target, int durationMs = 150);

    Variant m_variant;

    QColor m_baseColor;
    QColor m_hoverColor;
    QColor m_pressColor;
    QColor m_textColor;
    QColor m_borderColor;
    QColor m_currentColor;

    bool m_hovered = false;
    bool m_pressed = false;
    bool m_showFocusRing = false;

    QVariantAnimation *m_colorAnim;
};
