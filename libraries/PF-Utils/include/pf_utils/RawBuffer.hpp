#ifndef RAW_BUFFER_HPP
#define RAW_BUFFER_HPP

#include <cstddef>

namespace pf::util
{

/**
 * Stores:
 * - a pointer to the begining of the buffer
 * - size of the buffer (in bytes)
 * - count of the elements of type T stored in buffer
 *
 * Useful when you just need to pass this data to a C-style function, but you don't need to access
 * the data yourself.
 */
class RawBuffer
{
public:
    template <typename T>
    RawBuffer(T const *beginPointer, size_t count);

    template <typename ContainerType>
    explicit RawBuffer(ContainerType const &container);

    [[nodiscard]] void *pointer() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t count() const;

private:
    void *_pointer;
    size_t _size;
    size_t _count;
};


// * template definitions *

template <typename T>
RawBuffer::RawBuffer(T const *pointer, size_t count)
    : _pointer(reinterpret_cast<void *>(const_cast<T *>(pointer)))
    , _size(sizeof(T) * count)
    , _count(count)
{
}

template <typename ContainerType>
RawBuffer::RawBuffer(ContainerType const &container)
    : RawBuffer(container.data(), container.size())
{
}

} // namespace pf::util

#endif // !RAW_BUFFER_HPP
