#include "BiomeBag.hpp"
#include <algorithm>

const std::vector<eBiome> BiomeBag::s_biomes =
{
    eBiome::desert,
    eBiome::swamp,
    eBiome::forest,
    eBiome::rock,
    eBiome::ruin
};

BiomeBag::BiomeBag()
    : m_rng(std::random_device{}())
{
    refill();
}

eBiome BiomeBag::getRandomBiome()
{
    if (m_bag.empty())
    {
        refill();
    }

    eBiome biome = m_bag.back();
    m_bag.pop_back();

    return biome;
}

void BiomeBag::refill()
{
    m_bag = s_biomes;
    std::shuffle(m_bag.begin(), m_bag.end(), m_rng);
}