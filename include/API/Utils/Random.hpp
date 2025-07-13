#pragma once

#include "Concepts.hpp"

#include <random>

/**
 * @brief A utility class containing functions for generating various bits of psuedo-randomness
 * @note This class is not thread safe. Do not use this outside of the main thread within your plugin.
 * @note
 * This class does not do any security in regards to generation of random numbers.
 * You should not use this if you need to prevent RNGA (Random number generator attacks)
 */
class Random
{
#define Init                                           \
    if (!init)                                         \
    {                                                  \
        engine = std::mt19937(std::random_device{}()); \
        init = true;                                   \
    }

        inline static std::mt19937 engine;
        inline static bool init = false;

    public:
        Random() = delete;

        template <typename T>
        static T Uniform(T min, T max)
        {
            Init;
            return std::uniform_int_distribution<T>(min, max)(engine);
        }

        template <typename T>
            requires std::is_floating_point_v<T>
        static T UniformFloat(T min, T max)
        {
            Init;
            return std::uniform_real_distribution<T>(min, max)(engine);
        }

        template <typename T>
            requires StringRestriction<T>
        static T UniformString(unsigned length)
        {
            Init;

            std::uniform_int_distribution<short> aToZ('A', 'Z');
            T retStr;
            retStr.reserve(length);
            for (unsigned i = 0; i < length; i++)
            {
                retStr += static_cast<char>(aToZ(engine));
            }

            return retStr;
        }

        template <typename Iterator>
            requires std::random_access_iterator<Iterator>
        static unsigned Weighted(Iterator start, Iterator end)
        {
            Init;

            return std::discrete_distribution<>(start, end)(engine);
        }

        template <typename C>
            requires requires(C c) {
                { c.begin() } -> std::random_access_iterator;
                { c.end() } -> std::random_access_iterator;
            }
        static typename C::value_type Item(C container)
        {
            if (container.empty())
            {
                throw std::runtime_error("Container is empty");
            }

            return container[Uniform(0u, container.size() - 1u)];
        }

        static Vector RandomVector(float minRange, float maxRange)
        {
            return { UniformFloat(minRange, maxRange), UniformFloat(minRange, maxRange), UniformFloat(minRange, maxRange) };
        }

        static Matrix RandomMatrix()
        {
            return EulerMatrix(RandomVector(-180.f, 180.f));
        }
};

#undef Init
