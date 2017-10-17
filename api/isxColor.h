#ifndef ISX_COLOR_H
#define ISX_COLOR_H

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

        /// Verifies whether color is gray or not
        ///
        inline bool isGray() const
        {
            uint8_t r = (m_rgba >> 16) & 0xffu; // red
            uint8_t g = (m_rgba >>  8) & 0xffu; // green
            uint8_t b = (m_rgba      ) & 0xffu; // blue

            return ((r == g) && (r == b));
        }

    };

    /// Equality operator
    ///
    inline bool operator!=(const Color& lhs, const Color& rhs)
    {
        return (lhs.m_rgba != rhs.m_rgba);
    }

}

#endif // ISX_COLOR_H
