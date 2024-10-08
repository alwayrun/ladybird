/*
 * Copyright (c) 2022, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Utf8View.h>
#include <LibGfx/Font/Emoji.h>
#include <LibGfx/Font/ScaledFont.h>
#include <LibGfx/TextLayout.h>

namespace Gfx {

ScaledFont::ScaledFont(NonnullRefPtr<Typeface> typeface, float point_width, float point_height, unsigned dpi_x, unsigned dpi_y)
    : m_typeface(move(typeface))
    , m_point_width(point_width)
    , m_point_height(point_height)
{
    float units_per_em = m_typeface->units_per_em();
    m_x_scale = (point_width * dpi_x) / (POINTS_PER_INCH * units_per_em);
    m_y_scale = (point_height * dpi_y) / (POINTS_PER_INCH * units_per_em);

    auto metrics = m_typeface->metrics(m_x_scale, m_y_scale);

    m_pixel_size = m_point_height * (DEFAULT_DPI / POINTS_PER_INCH);
    m_pixel_size_rounded_up = static_cast<int>(ceilf(m_pixel_size));

    m_pixel_metrics = Gfx::FontPixelMetrics {
        .size = (float)pixel_size(),
        .x_height = metrics.x_height,
        .advance_of_ascii_zero = (float)glyph_width('0'),
        .ascent = metrics.ascender,
        .descent = metrics.descender,
        .line_gap = metrics.line_gap,
    };
}

float ScaledFont::width(StringView view) const { return measure_text_width(Utf8View(view), *this); }
float ScaledFont::width(Utf8View const& view) const { return measure_text_width(view, *this); }

float ScaledFont::glyph_width(u32 code_point) const
{
    auto string = String::from_code_point(code_point);
    return measure_text_width(Utf8View(string), *this);
}

template<typename CodePointIterator>
static float glyph_or_emoji_width_impl(ScaledFont const& font, CodePointIterator& it)
{
    if (auto const* emoji = Emoji::emoji_for_code_point_iterator(it))
        return font.pixel_size() * emoji->width() / emoji->height();

    return font.glyph_width(*it);
}

float ScaledFont::glyph_or_emoji_width(Utf8CodePointIterator& it) const
{
    return glyph_or_emoji_width_impl(*this, it);
}

NonnullRefPtr<ScaledFont> ScaledFont::scaled_with_size(float point_size) const
{
    if (point_size == m_point_height && point_size == m_point_width)
        return *const_cast<ScaledFont*>(this);
    return m_typeface->scaled_font(point_size);
}

NonnullRefPtr<Font> ScaledFont::with_size(float point_size) const
{
    return scaled_with_size(point_size);
}

Gfx::FontPixelMetrics ScaledFont::pixel_metrics() const
{
    return m_pixel_metrics;
}

float ScaledFont::pixel_size() const
{
    return m_pixel_size;
}

int ScaledFont::pixel_size_rounded_up() const
{
    return m_pixel_size_rounded_up;
}

float ScaledFont::point_size() const
{
    return m_point_height;
}

}
