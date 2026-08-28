#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include "eBiome.hpp"

class Voronoi {

public:
    struct Seed {
        int x = 0;
        int y = 0;
        eBiome type = eBiome::ocean;
        double influenceRadius = -1.0; // -1 = unlimited reach (used for ocean)
        bool hasChildren = false; // true if this seed comes from a nodesoup node with children
    };

    Voronoi(int width, int height, int seedCount, int seedMarkerRadius = 5);
    void generate(const std::vector<Seed>& biomeSeeds, int oceanSeedCount, double oceanMinDistFromBiome);
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const std::vector<std::vector<int>>& getImage() const { return image; }
    const std::vector<std::vector<int>>& getOwner() const { return owner; }
    const std::vector<Seed>& getSeeds() const { return seeds; }
    const std::vector<std::pair<int, int>>& getLandLandOceanJunctions() const { return junctions; }

private:
    int height;
    int width;
    int seed_count;
    int seed_marker_radius;
    std::vector<std::vector<int>> image;
    std::vector<std::vector<int>> owner;
    std::vector<Seed> seeds;
    std::vector<std::pair<int, int>> junctions; // points where 2 land biomes and ocean all meet

    void generate_ocean_seeds(const std::vector<Seed>& biomeSeeds, int count, double minDist);
    void fill_image(int color);
    void generate_random_seeds();
    void render_voronoi();
    void render_voronoi_biomes();
    void render_ocean();
    void render_seed_markers();
    void apply_coastal_noise(uint32_t noiseSeed, int coastBand = 14, double noiseFreq = 0.05);
    std::vector<std::pair<int, int>> detect_land_land_ocean_junctions() const;
    bool near_branch_node(int x, int y) const;
    void render_junction_markers(int markerRadius = 2);
    static int sqr_dist(int x1, int y1, int x2, int y2);
    static eBiome getBiomeType(int color);
};

