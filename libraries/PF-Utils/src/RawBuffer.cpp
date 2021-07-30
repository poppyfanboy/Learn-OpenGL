#include <pf_utils/RawBuffer.hpp>

namespace pf::util
{

void *RawBuffer::pointer() const
{
    return _pointer;
}

size_t RawBuffer::size() const
{
    return _size;
}

size_t RawBuffer::count() const
{
    return _count;
}

template <>
RawBuffer::RawBuffer(void const *pointer, size_t count)
    : _pointer(const_cast<void *>(pointer))
    , _size(count)
    , _count(count)
{
}

} // namespace pf::util
