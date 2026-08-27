#pragma once

#include <cstdint>
#include <cmath>

// Deterministic value noise. Everything here is a pure function of (x, y, seed),
// so the same seed always yields the same map - no global state, no srand.
namespace noise
{
    inline uint32_t hash2(int32_t x, int32_t y, uint32_t seed)
    {
        uint32_t h = seed + 0x9E3779B9u;
        h ^= static_cast<uint32_t>(x) * 0x85EBCA6Bu;
        h = (h << 13) | (h >> 19);
        h ^= static_cast<uint32_t>(y) * 0xC2B2AE35u;
        h *= 0x27D4EB2Fu;
        h ^= h >> 15;
        return h;
    }

    // Uniform [0,1) from a single integer key.
    inline double hash01(uint32_t key, uint32_t seed)
    {
        return hash2(static_cast<int32_t>(key), 0, seed) / 4294967296.0;
    }

    // Lattice value in [-1,1].
    inline double lattice(int32_t x, int32_t y, uint32_t seed)
    {
        return hash2(x, y, seed) / 2147483648.0 - 1.0;
    }

    inline double smoothstep(double t)
    {
        return t * t * (3.0 - 2.0 * t);
    }

    inline double value2d(double x, double y, uint32_t seed)
    {
        int32_t xi = static_cast<int32_t>(std::floor(x));
        int32_t yi = static_cast<int32_t>(std::floor(y));

        double u = smoothstep(x - xi);
        double v = smoothstep(y - yi);

        double a = lattice(xi,     yi,     seed);
        double b = lattice(xi + 1, yi,     seed);
        double c = lattice(xi,     yi + 1, seed);
        double d = lattice(xi + 1, yi + 1, seed);

        double top    = a + (b - a) * u;
        double bottom = c + (d - c) * u;

        return top + (bottom - top) * v;
    }

    // Fractal brownian motion: octaves of value2d at doubling frequency and
    // halving amplitude. This is what gives coastlines detail at several scales
    // instead of one uniform wobble.
    inline double fbm(double x, double y, uint32_t seed, int octaves,
                      double lacunarity = 2.0, double gain = 0.5)
    {
        double sum = 0.0;
        double amplitude = 1.0;
        double norm = 0.0;

        for (int i = 0; i < octaves; ++i)
        {
            sum  += amplitude * value2d(x, y, seed + static_cast<uint32_t>(i) * 1013u);
            norm += amplitude;

            x *= lacunarity;
            y *= lacunarity;
            amplitude *= gain;
        }

        return norm > 0.0 ? sum / norm : 0.0;
    }
}
