#pragma once

#include <QColor>
#include <QFont>

/**
 * @namespace Theme
 * @brief Central design-token repository — the Qt equivalent of globals.css.
 *
 * Organise tokens into sub-namespaces that mirror the CSS world:
 *   Theme::Color       → CSS custom properties for colour  (--color-primary …)
 *   Theme::Typography  → font families, sizes, weights     (font-size, font-weight …)
 *   Theme::Spacing     → layout dimensions                 (padding, gap, border-radius …)
 *   Theme::Animation   → motion timings                    (transition-duration …)
 *
 * All tokens are compile-time constants (inline constexpr / inline const).
 * Add new tokens here; never scatter magic numbers across widget files.
 */
namespace Theme
{

// ─────────────────────────────────────────────────────────────────────────────
// Color palette  (Tailwind-inspired, WCAG AA contrast ratios)
// Hex values encoded as {R, G, B} uint8 triplets.
// ─────────────────────────────────────────────────────────────────────────────
namespace Color
{
    // ── Primary (blue) ───────────────────────────────────────────────────────
    inline const QColor Blue300  {0x93, 0xC5, 0xFD}; ///< Focus ring / light accent
    inline const QColor Blue500  {0x3B, 0x82, 0xF6}; ///< Default / base
    inline const QColor Blue600  {0x25, 0x63, 0xEB}; ///< Hover
    inline const QColor Blue700  {0x1D, 0x4E, 0xD8}; ///< Active / pressed

    // ── Secondary (slate) ────────────────────────────────────────────────────
    inline const QColor Slate500 {0x64, 0x74, 0x8B}; ///< Default
    inline const QColor Slate600 {0x47, 0x55, 0x69}; ///< Hover
    inline const QColor Slate700 {0x33, 0x41, 0x55}; ///< Active / pressed

    // ── Danger (red) ─────────────────────────────────────────────────────────
    inline const QColor Red500   {0xEF, 0x44, 0x44}; ///< Default
    inline const QColor Red600   {0xDC, 0x26, 0x26}; ///< Hover
    inline const QColor Red700   {0xB9, 0x1C, 0x1C}; ///< Active / pressed

    // ── Neutral (gray) ───────────────────────────────────────────────────────
    inline const QColor Gray100  {0xF3, 0xF4, 0xF6}; ///< Surface / panel background
    inline const QColor Gray200  {0xE5, 0xE7, 0xEB}; ///< Disabled background
    inline const QColor Gray400  {0x9C, 0xA3, 0xAF}; ///< Disabled text / placeholder
    inline const QColor Gray500  {0x6B, 0x72, 0x80}; ///< Secondary text
    inline const QColor Gray700  {0x37, 0x41, 0x51}; ///< Body text
    inline const QColor Gray900  {0x11, 0x18, 0x27}; ///< Headings / high emphasis

    // ── Surface ──────────────────────────────────────────────────────────────
    inline const QColor White    {0xFF, 0xFF, 0xFF}; ///< Window / card background
} // namespace Color


// ─────────────────────────────────────────────────────────────────────────────
// Typography
// ─────────────────────────────────────────────────────────────────────────────
namespace Typography
{
    // Font sizes in points
    inline constexpr int SizeXs   =  9;
    inline constexpr int SizeSm   = 10;
    inline constexpr int SizeMd   = 11; ///< UI default / buttons
    inline constexpr int SizeLg   = 13;
    inline constexpr int SizeXl   = 16;
    inline constexpr int Size2Xl  = 18; ///< Page titles
    inline constexpr int Size3Xl  = 22;

    // Convenience font builders
    inline QFont body()
    {
        QFont f;
        f.setPointSize(SizeMd);
        return f;
    }

    inline QFont heading()
    {
        QFont f;
        f.setPointSize(Size2Xl);
        f.setBold(true);
        return f;
    }

    inline QFont caption()
    {
        QFont f;
        f.setPointSize(SizeSm);
        return f;
    }
} // namespace Typography


// ─────────────────────────────────────────────────────────────────────────────
// Spacing  (padding, gap, border-radius — all in logical pixels)
// ─────────────────────────────────────────────────────────────────────────────
namespace Spacing
{
    inline constexpr int Xs   =  4;
    inline constexpr int Sm   =  8;
    inline constexpr int Md   = 12;
    inline constexpr int Lg   = 16;
    inline constexpr int Xl   = 24;
    inline constexpr int Xxl  = 48;

    // Component-specific tokens
    inline constexpr int ButtonPaddingH  = 20; ///< Horizontal inner padding of buttons
    inline constexpr int ButtonPaddingV  = 10; ///< Vertical   inner padding of buttons
    inline constexpr int ButtonMinHeight = 44; ///< Accessible minimum tap target
    inline constexpr int ButtonMinWidth  = 130;

    inline constexpr int RadiusSm  =  4; ///< Badges, tags
    inline constexpr int RadiusMd  =  8; ///< Buttons, inputs (default)
    inline constexpr int RadiusLg  = 12; ///< Cards, panels
    inline constexpr int RadiusFull= 9999; ///< Pill / fully-rounded

    inline constexpr double BorderThin    = 1.0;
    inline constexpr double BorderDefault = 1.5;
    inline constexpr double BorderThick   = 2.0;
} // namespace Spacing


// ─────────────────────────────────────────────────────────────────────────────
// Animation  (durations in milliseconds — mirror CSS transition timings)
// ─────────────────────────────────────────────────────────────────────────────
namespace Animation
{
    inline constexpr int Fast    =  80; ///< Button press
    inline constexpr int Default = 150; ///< Hover in
    inline constexpr int Slow    = 200; ///< Hover out / fade
    inline constexpr int Xslow   = 300; ///< Page transitions, modals
} // namespace Animation

} // namespace Theme
