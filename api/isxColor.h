#ifndef ISX_COLOR_H
#define ISX_COLOR_H

#include <stdint.h>
#include <vector>
#include <utility>

namespace isx
{

    // Also, see QtGui/qrgb.h implementation
    // https://github.com/qt/qtbase/blob/dev/src/gui/painting/qrgb.h

    typedef uint32_t Rgba;

    /// Struct that defines Rgba color
    ///
    struct Color
    {
        /// uint color representation
        ///
        Rgba m_rgba;

        /// Default constructor which produce default (white) color
        ///
        Color() : m_rgba(0xffffffff) {}

        /// Constructor based uint
        ///
        Color(Rgba color) : m_rgba(color) {}

        /// Constructor based on channels
        ///
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            m_rgba = (a << 24) | (r << 16) | (g << 8) | (b);
        }

        /// \return The red value.
        ///
        inline uint8_t getRed() const
        {
            return uint8_t((m_rgba >> 16) & 0xffu);
        }

        /// \return The green value.
        ///
        inline uint8_t getGreen() const
        {
            return uint8_t((m_rgba >> 8) & 0xffu);
        }

        /// \return The blue value.
        ///
        inline uint8_t getBlue() const
        {
            return uint8_t(m_rgba & 0xffu);
        }

        /// Verifies whether color is gray or not
        ///
        inline bool isGray() const
        {
            const uint8_t r = getRed();
            return ((r == getGreen()) && (r == getBlue()));
        }
    };

    /// Equality operator
    ///
    inline bool operator!=(const Color& lhs, const Color& rhs)
    {
        return (lhs.m_rgba != rhs.m_rgba);
    }

    typedef std::pair<unsigned int, Color> IdColorPair;
    typedef std::vector<IdColorPair> IdColorPairs;
}

#endif // ISX_COLOR_H
