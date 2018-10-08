#include <collection/HashMap.h>

#include <random>

namespace Collection
{
I64HashFunctor::I64HashFunctor()
	: m_a()
	, m_b()
{
	Rehash();
}

void I64HashFunctor::Rehash()
{
	std::random_device device;
	std::mt19937_64 generator{ device() };
	std::uniform_int_distribution<int64_t> random;
	m_a = random(generator) | 1;
	m_b = random(generator) | 1;
}
}
