#include <pf_gl/RenderingOptions.hpp>

#include <stdexcept>
#include <string>

#include <sparsepp/spp.h>
#include <glad/glad.h>

namespace pf::gl
{

spp::sparse_hash_map<UsagePattern, GLenum> const USAGE_PATTERN_TO_GL_ENUM{
    {STATIC_DRAW, GL_STATIC_DRAW},
    {STATIC_READ, GL_STATIC_READ},
    {STATIC_COPY, GL_STATIC_COPY},
    {DYNAMIC_DRAW, GL_DYNAMIC_DRAW},
    {DYNAMIC_READ, GL_DYNAMIC_READ},
    {DYNAMIC_COPY, GL_DYNAMIC_COPY},
    {STREAM_DRAW, GL_STREAM_DRAW},
    {STREAM_READ, GL_STREAM_READ},
    {STREAM_COPY, GL_STREAM_COPY},
};

GLenum usagePatternToGLenum(UsagePattern bufferUsagePattern)
{
    if (USAGE_PATTERN_TO_GL_ENUM.find(bufferUsagePattern) == USAGE_PATTERN_TO_GL_ENUM.end())
    {
        throw std::invalid_argument("Unknown data usage pattern.");
    }

    return USAGE_PATTERN_TO_GL_ENUM.at(bufferUsagePattern);
}

} // namespace pf::gl
