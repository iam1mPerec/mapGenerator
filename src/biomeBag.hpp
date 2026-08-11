#pragma once

#include <vector>
#include <random>
#include "eBiome.hpp"

class BiomeBag
{
public:
    BiomeBag();

    eBiome getRandomBiome();

private:
    void refill();

private:
    static const std::vector<eBiome> s_biomes;

    std::vector<eBiome> m_bag;
    std::mt19937 m_rng;
};