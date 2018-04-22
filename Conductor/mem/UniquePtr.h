#pragma once

namespace Mem
{
template <typename T>
class UniquePtr
{
public:
	UniquePtr()
		: m_value(nullptr)
	{}

	UniquePtr(T* const value)
		: m_value(value)
	{}

	UniquePtr(const UniquePtr<T>&) = delete;
	UniquePtr& operator=(const UniquePtr<T>&) = delete;

	UniquePtr(UniquePtr<T>&& other)
		: m_value(other.Release())
	{}

	UniquePtr& operator=(UniquePtr<T>&& rhs)
	{
		Reset();
		m_value = rhs.Release();
		return *this;
	}

	template <typename ChildType>
	UniquePtr(UniquePtr<ChildType>&& other)
		: m_value(static_cast<T*>(other.Release()))
	{}

	template <typename ChildType>
	UniquePtr& operator=(UniquePtr<ChildType>&& rhs)
	{
		Reset();
		m_value = static_cast<T*>(rhs.Release());
		return *this;
	}

	~UniquePtr()
	{
		Reset();
	}

	void Reset()
	{
		if (m_value != nullptr)
		{
			delete m_value;
			m_value = nullptr;
		}
	}

	T* Get() { return m_value; }
	const T* Get() const { return m_value; }

	T* Release()
	{
		T* const out = m_value;
		m_value = nullptr;
		return out;
	}

	T& operator*() { return *m_value; }
	const T& operator*() const { return *m_value; }

	T* operator->() { return m_value; }
	const T* operator->() const { return m_value; }

	bool operator==(T* rhs) const { return (m_value == rhs); }
	bool operator==(const T* rhs) const { return (m_value == rhs); }
	bool operator==(std::nullptr_t rhs) const { return (m_value == rhs); }

	bool operator!=(T* rhs) const { return (m_value != rhs); }
	bool operator!=(const T* rhs) const { return (m_value != rhs); }
	bool operator!=(std::nullptr_t rhs) const { return (m_value != rhs); }

private:
	T* m_value;
};

template <typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Args&&... args)
{
	return UniquePtr<T>(new T(std::forward<Args>(args)...));
}
}
