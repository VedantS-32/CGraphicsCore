#pragma once
#include <cstdint>

namespace Cgr
{
    // To generate an invalid UUID, you can define a convention such as using 0 as an invalid value.
    // Example usage:

    // You may also add a helper method to the UUID class to check for validity:
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

		static UUID Invalid()
		{
			return UUID(0); // Return an invalid UUID
		}

        bool IsValid() const { return m_UUID != 0; } // Add this method

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