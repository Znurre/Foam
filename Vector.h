#ifndef VECTOR_H
#define VECTOR_H

#include <numeric>
#include <initializer_list>
#include <algorithm>
#include <memory>

template<typename T>
class immutable_vector
{
	public:
		immutable_vector()
			: m_size(0)
		{
		}

		immutable_vector(T &&value)
			: m_size(1)
			, m_storage(new T[1])
		{
			memcpy(m_storage.get(), &value, sizeof(T));
		}

		immutable_vector(const std::initializer_list<T> &&initializer)
			: m_size(initializer.size())
			, m_storage(new T[m_size])
		{
			std::copy(begin(initializer), end(initializer), m_storage.get());
		}

		template<typename ...TVector>
		immutable_vector(TVector &&... vectors)
			: m_size((vectors.size() + ...))
			, m_storage(new T[m_size])
		{
			copy(m_storage.get(), vectors...);
		}

		std::size_t size() const
		{
			return m_size;
		}

		T *data() const
		{
			return m_storage.get();
		}

	private:
		template<typename THead, typename ...TTail>
		static void copy(T *target, THead &&head, TTail &&...tail)
		{
			memcpy(target, head.data(), head.size() * sizeof(T));

			copy(target + head.size(), tail...);
		}

		template<typename THead>
		static void copy(T *target, THead &&head)
		{
			memcpy(target, head.data(), head.size() * sizeof(T));
		}

		std::size_t m_size;
		std::shared_ptr<T> m_storage;
};

#endif // VECTOR_H
