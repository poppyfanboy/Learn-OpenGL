#ifndef RENDERING_OPTIONS_HPP
#define RENDERING_OPTIONS_HPP

#include <glad/glad.h>

namespace pf::gl
{

enum UsagePattern
{
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,
    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,
    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY,
};

GLenum usagePatternToGLenum(UsagePattern bufferUsagePattern);

} // namespace pf::gl

#endif // !RENDERING_OPTIONS_HPP
