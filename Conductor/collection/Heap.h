#pragma once

#include <collection/ArrayView.h>
#include <collection/Vector.h>

namespace Collection
{
/**
 * A heap implemented as a binary tree.
 * MinHeap and MaxHeap define common heap properties, but a custom property can be provided.
 */
template <typename T, typename HeapProperty>
class Heap
{
	Vector<T> m_data;
	HeapProperty m_heapProperty;

public:
	using iterator = typename Vector<T>::iterator;
	using const_iterator = typename Vector<T>::const_iterator;

	explicit Heap(const HeapProperty& heapProperty = HeapProperty());
	explicit Heap(const ArrayView<const T>& elements, const HeapProperty& heapProperty = HeapProperty());

	bool IsEmpty() const { return m_data.IsEmpty(); }
	uint32_t Size() const { return m_data.Size(); }

	const T& Peek() const;
	T Pop();

	void Add(const T& e);
	void Add(T&& e);

	T PopAdd(const T& e);
	T PopAdd(T&& e);

	void NotifyElementChanged(iterator iter);

	iterator begin() { return m_data.begin(); }
	const_iterator begin() const { return m_data.begin(); }
	const_iterator cbegin() const { return m_data.cbegin(); }

	iterator end() { return m_data.end(); }
	const_iterator end() const { return m_data.end(); }
	const_iterator cend() const { return m_data.cend(); }

private:
	void SiftUp(const size_t startIndex);
	void SiftDown(const size_t startIndex);
};

template <typename T>
struct MinHeapProperty
{
	bool Test(const T& parent, const T& child) const { return parent <= child; }
};

template <typename T>
struct MaxHeapProperty
{
	bool Test(const T& parent, const T& child) const { return parent >= child; }
};

template <typename T>
using MinHeap = Heap<T, MinHeapProperty<T>>;

template <typename T>
using MaxHeap = Heap<T, MaxHeapProperty<T>>;
}

// Inline implementations.
namespace Collection
{
template <typename T, typename HeapProperty>
inline Heap<T, HeapProperty>::Heap(const HeapProperty& heapProperty)
	: m_data()
	, m_heapProperty(heapProperty)
{}

template <typename T, typename HeapProperty>
inline Heap<T, HeapProperty>::Heap(const ArrayView<const T>& elements, const HeapProperty& heapProperty)
	: m_data(elements)
	, m_heapProperty(heapProperty)
{
	for (int64_t i = m_data.Size() - 1; i > 0; --i)
	{
		SiftDown(i);
	}
}

template <typename T, typename HeapProperty>
inline const T& Heap<T, HeapProperty>::Peek() const
{
	return m_data.Front();
}

template <typename T, typename HeapProperty>
inline T Heap<T, HeapProperty>::Pop()
{
	T out = std::move(m_data.Front());

	m_data.Front() = std::move(m_data.Back());
	m_data.RemoveLast();
	SiftDown(0);

	return out;
}

template <typename T, typename HeapProperty>
inline void Heap<T, HeapProperty>::Add(const T& e)
{
	m_data.Add(e);
	SiftUp(m_data.Size() - 1);
}

template <typename T, typename HeapProperty>
inline void Heap<T, HeapProperty>::Add(T&& e)
{
	m_data.Add(std::move(e));
	SiftUp(m_data.Size() - 1);
}

template <typename T, typename HeapProperty>
inline T Heap<T, HeapProperty>::PopAdd(const T& e)
{
	T out = std::move(m_data.Front());

	m_data.Front() = e;
	SiftDown(0);

	return out;
}

template <typename T, typename HeapProperty>
inline T Heap<T, HeapProperty>::PopAdd(T&& e)
{
	T out = std::move(m_data.Front());

	m_data.Front() = std::move(e);
	SiftDown(0);

	return out;
}

template <typename T, typename HeapProperty>
inline void Heap<T, HeapProperty>::NotifyElementChanged(iterator iter)
{
	const int64_t index = std::distance(begin(), iter);
	// Only one of SiftUp or SiftDown will actually move the element.
	SiftUp(index);
	SiftDown(index);
}

template <typename T, typename HeapProperty>
inline void Heap<T, HeapProperty>::SiftUp(const size_t startIndex)
{
	size_t index = startIndex;
	while (index > 0)
	{
		const size_t parentIndex = (index - 1) / 2;
		if (m_heapProperty.Test(m_data[parentIndex], m_data[index]))
		{
			// TODO is this accurate?
			break;
		}

		using std::swap;
		swap(m_data[parentIndex], m_data[index]);

		index = parentIndex;
	}
}

template <typename T, typename HeapProperty>
inline void Heap<T, HeapProperty>::SiftDown(const size_t startIndex)
{
	size_t index = startIndex;
	size_t leftChildIndex = (index * 2) + 1;
	while (leftChildIndex < m_data.Size())
	{
		const size_t rightChildIndex = leftChildIndex + 1;
		const size_t bestChildIndex = (rightChildIndex >= m_data.Size())
			? leftChildIndex
			: (m_heapProperty.Test(m_data[leftChildIndex], m_data[rightChildIndex])
				? leftChildIndex
				: rightChildIndex);

		if (m_heapProperty.Test(m_data[index], m_data[bestChildIndex]))
		{
			// TODO is this accurate?
			break;
		}

		using std::swap;
		swap(m_data[index], m_data[bestChildIndex]);

		index = bestChildIndex;
	}
}
}
