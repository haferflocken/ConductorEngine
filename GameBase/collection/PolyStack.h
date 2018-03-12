#pragma once

#include <collection/PolyBuffer.h>

#include <type_traits>

namespace Collection
{
template <typename BaseType>
class PolyStack
{
public:
	template <typename T>
	using IsConvertibleType = std::enable_if_t<std::is_convertible_v<T*, BaseType*>, T>;

	explicit PolyStack(const Unit::ByteCount64 initialCapacityInBytes = Unit::WordCount32(8))
		: m_buffer(initialCapacityInBytes)
	{}

	bool IsEmpty() const { return m_buffer.empty(); }

	BaseType* Peek() { return m_buffer.peek<BaseType>(); }
	const BaseType* Peek() const { return m_buffer.peek<BaseType>(); }

	template <typename T, typename... Args>
	IsConvertibleType<T>& Emplace(Args&&... args);

	void Pop() { m_buffer.pop(); }

private:
	PolyBuffer m_buffer;
};

template <typename BaseType>
template <typename T, typename... Args>
inline PolyStack<BaseType>::IsConvertibleType<T>& PolyStack<BaseType>::Emplace(Args&&... args)
{
	while (!m_buffer.hasRoomFor<T>())
	{
		// If we need more room, double our capacity.
		const Unit::WordCount32 newCapacity = m_buffer.capacity() * 2;
		PolyBuffer newBuffer{ newCapacity };

		// Move the contents of the old buffer into the new one.
		m_buffer.memcpyMoveAll(newBuffer);
		
		// Move the new buffer into m_buffer, deleting the old buffer in the process.
		m_buffer = std::move(newBuffer);
	}
	return m_buffer.emplace<T>(std::forward<Args>(args)...);
}
}
