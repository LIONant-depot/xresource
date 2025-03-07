#ifndef XRESOURCE_GUID_H
#define XRESOURCE_GUID_H
#pragma once

#include <cstdint>  // uint64_t
#include <cassert>  // assert
#include <thread>   // std::thread::id
#include <random>   // std::random_device, std::mt19937_64, std::uniform_int_distribution
#include <chrono>   // std::chrono::system_clock, std::chrono::duration_cast, std::chrono::seconds
#include <map>      // std::hash

// This header is meant to solve the problem for unique identifiers for resources
// The main concept here is that we define one GUID (Global Unique Identifier) as two elements...
// The type which specifies what type of resource it is with a unique identifier
// and the Instance which identifies which actual resource we are talking about
// The type should be fairly confortable with 64bits not matter what...
// But the instance depending of the problem you are trying to solve may require to be larger
// For that porpouse we hace provided a _large version of the instance which is 128bits which
// is the standard for commercial products (overkilled for smaller projects)
// The def_guid is typically what most project will use in their components since the type will be fixed

namespace xresource
{
    struct details
    {
        // This is a struct instead of a namespace because I need the functions not to inline yet still be in the header files
        details() = delete;

        //------------------------------------------------------------------------------------------------

        static __declspec(noinline) [[nodiscard]] std::uint64_t GenerateRSCTypeGUID64() noexcept
        {
            // Thread-local RNG and counter
            thread_local std::mt19937_64                            rng(std::random_device{}());
            thread_local std::uniform_int_distribution<uint64_t>    dist;

            // 14-bit counter per thread per second... if we generate more than 16383 GUIDs per second, we have a problem
            thread_local std::uint16_t  counter             = 0;

            // Thread-specific identifier (8 bits from thread ID hash)
            thread_local auto           threadSalt          = static_cast<uint16_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()) & 0xFF);

            // Machine-specific salt (12 bits, generated once per process)
            static const auto           machineSalt         = static_cast<uint16_t>(std::random_device{}() & 0xFFF);

            // Timestamp in seconds since epoch (28 bits, ~8.5 years) starting date Jan 1, 2025 UTC
            static const auto           epoch               = std::chrono::system_clock::from_time_t(1735689600); 
            const auto                  timestamp           = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - epoch).count();
            const auto                  timeComponent       = static_cast<uint64_t>(timestamp & 0xFFFFFFF);

            // Generate a 6-bit random component
            const auto                  randomComponent     = static_cast<uint8_t>(dist(rng) & 0x3F);

            // Combine: 28 bits timestamp + 12 bits machine + 8 bits thread + 14 bits counter + 6 bits random
            std::uint64_t guid = (timeComponent << 36)                              // 28 bits: timestamp
                                | (static_cast<std::uint64_t>(machineSalt) << 24)   // 12 bits: machine salt
                                | (static_cast<std::uint64_t>(threadSalt) << 16)    //  8 bits: thread ID
                                | ((counter++ & 0x3FFF) << 6)                       // 14 bits: counter (0-16383)
                                | randomComponent;                                  //  6 bits: random (0-63)

            return guid;
        }

        //------------------------------------------------------------------------------------------------

        static __declspec(noinline) [[nodiscard]] uint64_t GenerateRSCInstanceGUID64() noexcept
        {
            // Thread-local RNG and counter
            thread_local std::mt19937_64                            rng(std::random_device{}());
            thread_local std::uniform_int_distribution<uint64_t>    dist;

            // 14-bit counter per thread per second... if we generate more than 16383 GUIDs per second, we have a problem
            thread_local std::uint16_t  counter         = 0; 

            // Thread-specific identifier (8 bits from thread ID hash)
            thread_local auto           threadSalt      = static_cast<uint16_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()) & 0xFF);

            // Machine-specific salt (12 bits, generated once per process)
            static const auto           machineSalt     = static_cast<uint16_t>(std::random_device{}() & 0xFFF);

            // Timestamp in seconds since epoch (28 bits, ~8.5 years) starting Jan 1, 2025 UTC
            static const auto           epoch           = std::chrono::system_clock::from_time_t(1735689600);
            const auto                  timestamp       = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - epoch).count();
            const auto                  timeComponent   = static_cast<std::uint64_t>(timestamp & 0xFFFFFFF); // 28 bits

            // Generate a 5-bit random component
            const auto                  randomComponent = static_cast<uint8_t>(dist(rng) & 0x1F);

            // Combine: 28 bits timestamp + 12 bits machine + 8 bits thread + 14 bits counter + 5 bits random, shifted left by 1
            std::uint64_t guid =  (timeComponent << 37)                                 // 28 bits: timestamp (bits 63-37)
                                | (static_cast<std::uint64_t>(machineSalt) << 25)       // 12 bits: machine salt (bits 36-25)
                                | (static_cast<std::uint64_t>(threadSalt) << 17)        // 8 bits: thread ID (bits 24-17)
                                | ((counter++ & 0x3FFF) << 6)                           // 14 bits: counter (bits 19-6)
                                | (static_cast<std::uint64_t>(randomComponent) << 1)    // 5 bits: random (bits 5-1)
                                | 1;                                                    // Bit 0 is left as 1 as a requirement for instance GUIDs

            return guid;
        }

        //------------------------------------------------------------------------------------------------

        static __declspec(noinline) [[nodiscard]] std::pair<std::uint64_t, std::uint64_t> GenerateRSCInstanceGUID128() noexcept
        {
            // Thread-local RNG and counter
            thread_local std::mt19937_64                            rng(std::random_device{}());
            thread_local std::uniform_int_distribution<uint64_t>    dist;

            // 28-bit counter per thread per second (268M possible values)
            thread_local std::uint32_t  counter         = 0;

            // Thread-specific identifier (20 bits from thread ID hash)
            thread_local auto           threadSalt      = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()) & 0xFFFFF);

            // Machine-specific salt (20 bits, generated once per process)
            static const auto           machineSalt     = static_cast<uint32_t>(std::random_device{}() & 0xFFFFF);

            // Timestamp in seconds since epoch (31 bits, ~68 years) starting Jan 1, 2025 UTC
            static const auto           epoch           = std::chrono::system_clock::from_time_t(1735689600);
            const auto                  timestamp       = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - epoch).count();
            const auto                  timeComponent   = static_cast<std::uint64_t>(timestamp & 0x7FFFFFFF); // 31 bits

            // Generate a 28-bit random component
            const auto                  randomComponent = static_cast<std::uint32_t>(dist(rng) & 0xFFFFFFF);

            // Split into two 64-bit parts
            // High 64 bits: 31 bits timestamp + 20 bits machine + 13 bits thread
            std::uint64_t high = (timeComponent << 33)                                      // 31 bits: timestamp (bits 63-33)
                               | (static_cast<std::uint64_t>(machineSalt) << 13)            // 20 bits: machine salt (bits 32-13)
                               | (threadSalt >> 7);                                         // 13 bits: upper part of thread ID (bits 12-0)

            // Low 64 bits: 7 bits thread + 28 bits counter + 28 bits random + 1 bit flag
            std::uint64_t low = (static_cast<std::uint64_t>(threadSalt & 0x7F) << 57)       // 7 bits: lower part of thread ID (bits 63-57)
                              | (static_cast<std::uint64_t>(counter++ & 0xFFFFFFF) << 29)   // 28 bits: counter (bits 56-29)
                              | (static_cast<std::uint64_t>(randomComponent) << 1)          // 28 bits: random (bits 28-1)
                              | 1;                                                          // Bit 0 is left as 1

            return { low, high };
        }
    };

    //------------------------------------------------------------------------------------------------

    template<typename T_ARG>
    struct guid
    {
        std::uint64_t m_Value;

        constexpr
        [[nodiscard]]bool operator == (const guid& B) const noexcept
        {
            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
            {
                assert(isValid() || empty());
                assert(B.isValid() || B.empty());
            }
            return m_Value == B.m_Value;
        }

        constexpr
        [[nodiscard]]bool operator != (const guid& B) const noexcept
        {
            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
            {
                assert(isValid() || empty());
                assert(B.isValid() || B.empty());
            }
            return m_Value != B.m_Value;
        }

        constexpr
       [[nodiscard]] bool operator < (const guid& B) const noexcept
        {
            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
            {
                assert(isValid() || empty());
                assert(B.isValid() || B.empty());
            }
            return m_Value < B.m_Value;
        }

        constexpr
        [[nodiscard]]bool operator > (const guid& B) const noexcept
        {
            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
            {
                assert(isValid() || empty());
                assert(B.isValid() || B.empty());
            }
            return m_Value > B.m_Value;
        }

        constexpr 
        [[nodiscard]] bool empty() const noexcept
        {
            return m_Value == 0;
        }

        inline void clear() noexcept
        {
            m_Value = 0;
        }

        constexpr
        [[nodiscard]] bool isValid() const noexcept requires std::is_same_v<T_ARG, struct rsc_instance_guid_tag>
        {
            return !!(m_Value & 1);
        }

        inline static [[nodiscard]] guid GenerateGUID() noexcept
        {
            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
                return {details::GenerateRSCInstanceGUID64()};
            else
                return {details::GenerateRSCTypeGUID64()};
        }

        // Generate GUID based on a string... we try to make it well distributed in the space of bits rather than unique in time
        static inline
        [[nodiscard]] guid GenerateGUID(const char* str, uint64_t hash = 0xcbf29ce484222325ULL) noexcept
        {
            // MurmurHash3 (64-bit)
            const uint64_t m = 0xc6a4a7935bd1e995ULL;
            int len = 0;

            const unsigned char* data = reinterpret_cast<const unsigned char*>(str);
            while (*data) 
            {
                hash ^= static_cast<uint64_t>(*data++);
                hash *= m;
                len++;
            }

            // Finalization
            hash ^= len;
            hash ^= hash >> 33;
            hash *= 0x85ebca6b;
            hash ^= hash >> 13;
            hash *= 0xc2b2ae35;
            hash ^= hash >> 16;

            if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
                return {(hash<<1) | 1};
            else
                return {hash};
        }

        // Compile time version of GenerateGUID for strings
        static consteval
        [[nodiscard]] guid generate_guid(const char* str, uint64_t hash = 0x548c9decbce65297ULL, int len = 0) noexcept
        {
            constexpr uint64_t m = 0xc6a4a7935bd1e995ULL;
            uint64_t h = hash;

            // Process string character-by-character
            return (*str == '\0') ?
                // Finalization when string ends
                [](uint64_t h, int len) consteval noexcept -> guid
                {
                    h ^= len;
                    h ^= h >> 33;
                    h *= 0x85ebca6b;
                    h ^= h >> 13;
                    h *= 0xc2b2ae35;
                    h ^= h >> 16;

                    if constexpr (std::is_same_v<T_ARG, struct rsc_instance_guid_tag>)
                        return { (h << 1) | 1 };
                    else
                        return { h };
                }(h, len) :
                // Continue hashing
                generate_guid(str + 1, (h ^ static_cast<uint64_t>(*str)) * m, len + 1);
        }
    };

    //------------------------------------------------------------------------------------------------

    struct instance_guid_large
    {
        std::uint64_t m_Low;
        std::uint64_t m_High;

        constexpr
        bool operator == (const instance_guid_large& B) const noexcept
        {
            assert(isValid() || empty());
            assert(B.isValid() || B.empty());
            return m_Low == B.m_Low && m_High == B.m_High;
        }

        constexpr
        bool operator != (const instance_guid_large& B) const noexcept
        {
            assert(isValid() || empty());
            assert(B.isValid() || B.empty());
            return m_Low != B.m_Low || m_High != B.m_High;
        }

        constexpr
        bool operator < (const instance_guid_large& B) const noexcept
        {
            assert(isValid() || empty());
            assert(B.isValid() || B.empty());
            return m_Low < B.m_Low || (m_Low == B.m_Low && m_High < B.m_High);
        }

        constexpr
        bool operator > (const instance_guid_large& B) const noexcept
        {
            assert(isValid() || empty());
            assert(B.isValid() || B.empty());
            return m_Low > B.m_Low || (m_Low == B.m_Low && m_High > B.m_High);
        }

        constexpr 
        [[nodiscard]] bool empty() const noexcept
        {
            return m_Low == 0 && m_High==0;
        }

        inline void clear() noexcept
        {
            m_Low = m_High = 0;
        }

        constexpr
        [[nodiscard]] bool isValid() const noexcept
        {
            return (m_Low & 1) == 1;
        }

        inline static [[nodiscard]] instance_guid_large GenerateGUID() noexcept
        {
            const auto P = details::GenerateRSCInstanceGUID128();
            return { P.first, P.second };
        }

        static inline
        [[nodiscard]] instance_guid_large GenerateGUID(const char* str, uint64_t seed1 = 0x548c9decbce65297ULL, uint64_t seed2 = 0x548c9decbce65297ULL) noexcept
        {
            // Constants for MurmurHash3 128-bit variant
            constexpr uint64_t m1 = 0x87c37b91114253d5ULL;
            constexpr uint64_t m2 = 0x4cf5ad432745937fULL;

            uint64_t h1 = seed1;  // Will become high 64 bits
            uint64_t h2 = seed2;  // Will become low 64 bits
            int len = 0;

            const unsigned char* data = reinterpret_cast<const unsigned char*>(str);

            // Process the string 16 bytes at a time (128 bits)
            while (*data) 
            {
                uint64_t k1 = 0;
                uint64_t k2 = 0;

                // Read up to 16 bytes (8 for k1, 8 for k2)
                for (int i = 0; i < 8 && *data; i++) 
                {
                    k1 |= static_cast<uint64_t>(*data++) << (i * 8);
                    len++;
                }
                for (int i = 0; i < 8 && *data; i++) 
                {
                    k2 |= static_cast<uint64_t>(*data++) << (i * 8);
                    len++;
                }

                // Mix k1 into h1
                k1 *= m1;
                k1 ^= k1 >> 33;
                k1 *= m2;
                h1 ^= k1;

                // Mix k2 into h2
                k2 *= m2;
                k2 ^= k2 >> 33;
                k2 *= m1;
                h2 ^= k2;

                // Mix h1 and h2
                h1 ^= h2;
                h2 ^= h1;
            }

            // Finalization
            h1 ^= len;
            h2 ^= len;

            h1 += h2;
            h2 += h1;

            h1 ^= h1 >> 33;
            h1 *= 0x85ebca6b;
            h1 ^= h1 >> 13;
            h1 *= 0xc2b2ae35;
            h1 ^= h1 >> 16;

            h2 ^= h2 >> 33;
            h2 *= 0x85ebca6b;
            h2 ^= h2 >> 13;
            h2 *= 0xc2b2ae35;
            h2 ^= h2 >> 16;

            h1 += h2;
            h2 += h1;

            // For instance GUIDs, set the least significant bit to 1
            h2 = (h2 << 1) | 1;

            return instance_guid_large{ h2, h1 };
        }

        static consteval
        [[nodiscard]] instance_guid_large generate_guid(const char* str, uint64_t h1 = 0x548c9decbce65297ULL, uint64_t h2 = 0x548c9decbce65297ULL, int len = 0) noexcept
        {
            constexpr uint64_t m1 = 0x87c37b91114253d5ULL;  // Multiplication constant for h1
            constexpr uint64_t m2 = 0x4cf5ad432745937fULL;  // Multiplication constant for h2

            // Process string character-by-character
            return (*str == '\0') ?
                // Finalization when string ends
                [](uint64_t h1, uint64_t h2, int len) consteval noexcept -> instance_guid_large
                {
                    // Finalization mix for h1
                    h1 ^= len;
                    h1 ^= h1 >> 33;
                    h1 *= 0x85ebca6b;
                    h1 ^= h1 >> 13;
                    h1 *= 0xc2b2ae35;
                    h1 ^= h1 >> 16;

                    // Finalization mix for h2
                    h2 ^= len;
                    h2 ^= h2 >> 33;
                    h2 *= 0x85ebca6b;
                    h2 ^= h2 >> 13;
                    h2 *= 0xc2b2ae35;
                    h2 ^= h2 >> 16;

                    // Additional mixing between h1 and h2
                    h1 += h2;
                    h2 += h1;

                    // Set LSB to 1 for instance GUID
                    h2 = (h2 << 1) | 1;

                    return instance_guid_large{h1, h2};
                }(h1, h2, len) :
                // Continue hashing: alternate between affecting h1 and h2
                (len % 2 == 0) ?
                    generate_guid(str + 1, 
                                 (h1 ^ static_cast<uint64_t>(*str)) * m1,  // Mix current char into h1
                                 h2,
                                 len + 1) :
                    generate_guid(str + 1,
                                 h1,
                                 (h2 ^ static_cast<uint64_t>(*str)) * m2,  // Mix current char into h2
                                 len + 1);
        }
    };

    //------------------------------------------------------------------------------------------------

    using instance_guid     = guid< struct rsc_instance_guid_tag >;
    using type_guid         = guid< struct rsc_type_guid_tag >;

    //------------------------------------------------------------------------------------------------

    template< typename T_INSTANCE_TYPE, type_guid T_TYPE_GUID_V >
    struct def_guid_t
    {
        T_INSTANCE_TYPE                 m_Instance;     // also used in the pool as the next empty entry...
        inline static constexpr auto    m_Type      = T_TYPE_GUID_V;

        template< typename T>
        constexpr bool operator == (const T& B) const noexcept
        {
            return m_Instance == B.m_Instance && m_Type == B.m_Type;
        }

        constexpr bool empty() const noexcept
        {
            return m_Instance.m_Value == 0;
        }

        void clear() noexcept
        {
            m_Instance.m_Value = 0;
        }

        constexpr bool isValid() const noexcept
        {
            return m_Instance.isValid();
        }
    };

    //------------------------------------------------------------------------------------------------
    template< typename type_guid T_TYPE_GUID_V >
    using def_guid = def_guid_t<instance_guid, T_TYPE_GUID_V>;

    template< typename type_guid T_TYPE_GUID_V >
    using def_guid_large = def_guid_t<instance_guid_large, T_TYPE_GUID_V>;

    //------------------------------------------------------------------------------------------------

    template< typename T_INSTANCE_TYPE >
    struct full_guid_t
    {
        T_INSTANCE_TYPE         m_Instance;
        type_guid               m_Type;

        constexpr bool operator == (const full_guid_t& B) const noexcept
        {
            return m_Instance == B.m_Instance && m_Type == B.m_Type;
        }

        constexpr bool operator != (const full_guid_t& B) const noexcept
        {
            return m_Instance != B.m_Instance || m_Type != B.m_Type;
        }

        constexpr bool empty() const noexcept
        {
            return m_Instance.empty();
        }

        void clear() noexcept
        {
            m_Instance.clear();
            m_Type.clear();
        }

        constexpr bool isValid() const noexcept
        {
            return m_Instance.isValid();
        }
    };

    //------------------------------------------------------------------------------------------------
    
    using full_guid       = full_guid_t<instance_guid>;
    using full_guid_large = full_guid_t<instance_guid_large>;

    //------------------------------------------------------------------------------------------------
    
    template< typename T >
    inline static std::size_t ComputeHash(guid<T> k) noexcept
    {
        return std::hash<std::uint64_t>{}(k.m_Value);
    }

    //------------------------------------------------------------------------------------------------

    inline
    static std::size_t ComputeHash(const full_guid& k) noexcept
    {
        // Use a combination of the two uint64_t values
        std::size_t Value = std::hash<std::uint64_t>{}(k.m_Instance.m_Value) ^ (std::hash<std::uint64_t>{}(k.m_Type.m_Value) << 1);
        return Value;
    }

    //------------------------------------------------------------------------------------------------

    template<type_guid T_TYPE_GUID_V >
    inline
    static std::size_t ComputeHash(const def_guid<T_TYPE_GUID_V>& k) noexcept
    {
        // Use a combination of the two uint64_t values
        std::size_t Value = std::hash<std::uint64_t>{}(k.m_Instance) ^ (std::hash<std::uint64_t>{}(k.m_Type) << 1);
        return Value;
    }
}

//------------------------------------------------------------------------------------------------
// Support for std::hash
//------------------------------------------------------------------------------------------------
namespace std
{
    template<>
    struct hash<xresource::instance_guid>
    {
        inline
        std::size_t operator()(const xresource::instance_guid& k) const noexcept
        {
            return std::hash<std::uint64_t>{}(k.m_Value);
        }
    };
    template<>
    struct hash<xresource::type_guid>
    {
        inline
        std::size_t operator()(const xresource::type_guid& k) const noexcept
        {
            return std::hash<std::uint64_t>{}(k.m_Value);
        }
    };
    template<>
    struct hash<xresource::instance_guid_large>
    {
        inline
        std::size_t operator()(const xresource::instance_guid_large& k) const noexcept
        {
            std::size_t h1 = std::hash<uint64_t>{}(k.m_Low);
            std::size_t h2 = std::hash<uint64_t>{}(k.m_High);

            // Combine the hashes (simple but effective method)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    template<>
    struct hash<xresource::full_guid>
    {
        inline
        std::size_t operator()(const xresource::full_guid& k) const noexcept
        {
            std::size_t h1 = std::hash<uint64_t>{}(k.m_Instance.m_Value);
            std::size_t h2 = std::hash<uint64_t>{}(k.m_Type.m_Value);

            // Combine the hashes (simple but effective method)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    template<>
    struct hash<xresource::full_guid_large>
    {
        inline
        std::size_t operator()(const xresource::full_guid_large& k) const noexcept
        {
            std::size_t h1 = std::hash<xresource::instance_guid_large>{}(k.m_Instance);
            std::size_t h2 = std::hash<uint64_t>{}(k.m_Type.m_Value);

            // Combine the hashes (simple but effective method)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    template< xresource::type_guid T_TYPE_GUID_V >
    struct hash<xresource::def_guid<T_TYPE_GUID_V>>
    {
        inline
        std::size_t operator()(const xresource::def_guid<T_TYPE_GUID_V>& k) const noexcept
        {
            std::size_t h1 = std::hash<uint64_t>{}(k.m_Instance.m_Value);
            std::size_t h2 = std::hash<uint64_t>{}(k.m_Type.m_Value);

            // Combine the hashes (simple but effective method)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    template< xresource::type_guid T_TYPE_GUID_V >
    struct hash<xresource::def_guid_large<T_TYPE_GUID_V>>
    {
        inline
        std::size_t operator()(const xresource::def_guid_large<T_TYPE_GUID_V>& k) const noexcept
        {
            std::size_t h1 = std::hash<xresource::instance_guid_large>{}(k.m_Instance);
            std::size_t h2 = std::hash<uint64_t>{}(k.m_Type.m_Value);

            // Combine the hashes (simple but effective method)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}

#endif