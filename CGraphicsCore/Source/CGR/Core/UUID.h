#pragma once
#include <cstdint>

namespace Cgr
{
	class CGR_API UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

		const void* ValuePtr() const {
			return &m_UUID;
		}

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Cgr::UUID>
	{
		size_t operator()(const Cgr::UUID& uuid) const
		{
			return uint64_t(uuid);
		}
	};
}