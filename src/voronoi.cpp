#include <time.h>
#include <queue>
#include <unordered_set>
#include "voronoi.hpp"
#include "colors.hpp"
#include "noise.hpp"

Voronoi::Voronoi(int w, int h, int sc, int mr)
    : width(w), height(h), seed_count(sc), seed_marker_radius(mr)
{
    image.assign(height, std::vector<int>(width, color::BLACK));
    owner.assign(height, std::vector<int>(width, 0));
    //seeds.resize(seed_count);
}

void Voronoi::generate_ocean_seeds(const std::vector<Seed>& biomeSeeds, int count, double minDist)
{
    double minDistSq = minDist * minDist;
    int attempts = 0;
    int maxAttempts = count * 500; // safety valve if space is too crowded

    int placed = 0;
    while (placed < count && attempts < maxAttempts)
    {
        attempts++;
        int x = rand() % width;
        int y = rand() % height;

        bool tooClose = false;
        for (auto& bs : biomeSeeds)
        {
            if (sqr_dist(x, y, bs.x, bs.y) < minDistSq)
            {
                tooClose = true;
                break;
            }
        }
        if (tooClose)
            continue;

        Seed s;
        s.x = x;
        s.y = y;
        s.type = eBiome::ocean;
        s.influenceRadius = -1.0;
        seeds.push_back(s);
        placed++;
    }
}

void Voronoi::fill_image(int color)
{
    for (auto& row : image)
        std::fill(row.begin(), row.end(), color);
}

int Voronoi::sqr_dist(int x1, int y1, int x2, int y2)
{
    int dx = x1 - x2;
    int dy = y1 - y2;
    return dx * dx + dy * dy;
}

void Voronoi::generate_random_seeds()
{
    for (auto& seed : seeds)
    {
        seed.x = rand() % width;
        seed.y = rand() % height;
        seed.type = eBiome::ocean;
    }
}

void Voronoi::render_voronoi()
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int best_seed = 0;
            int best_dist = sqr_dist(seeds[0].x, seeds[0].y, x, y);

            for (int i = 1; i < seed_count; ++i)
            {
                int d = sqr_dist(seeds[i].x, seeds[i].y, x, y);
                if (d < best_dist)
                {
                    best_dist = d;
                    best_seed = i;
                }
            }

            owner[y][x] = best_seed;
            image[y][x] = palette[best_seed % palette_count];
        }
    }
}

void Voronoi::render_voronoi_biomes()
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int best_seed = -1;
            double best_dist = std::numeric_limits<double>::max();

            for (size_t i = 0; i < seeds.size(); ++i)
            {
                double d = sqr_dist(seeds[i].x, seeds[i].y, x, y);

                if (seeds[i].influenceRadius >= 0.0)
                {
                    double capSq = seeds[i].influenceRadius * seeds[i].influenceRadius;
                    if (d > capSq)
                        continue; // out of reach for this pixel, skip it entirely
                }

                if (d < best_dist)
                {
                    best_dist = d;
                    best_seed = (int)i;
                }
            }

            if (best_seed == -1)
            {
                // nothing reaches this pixel (shouldn't happen as long as ocean seeds exist)
                owner[y][x] = -1;
                image[y][x] = color::BLACK;
            }
            else
            {
                owner[y][x] = best_seed;
                image[y][x] = biomeToColor(seeds[best_seed].type);
            }
        }
    }
}

eBiome Voronoi::getBiomeType(int color)
{
    if (color == color::GRAY)   return eBiome::ruin;
    if (color == color::GREEN)  return eBiome::forest;
    if (color == color::YELLOW) return eBiome::desert;
    if (color == color::PURPLE) return eBiome::swamp;
    if (color == color::BLUE)   return eBiome::rock;
    return eBiome::ocean;
}

void Voronoi::render_ocean()
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int seed_idx = owner[y][x];
            seeds[seed_idx].type = getBiomeType(image[y][x]);
        }
    }
}

void Voronoi::apply_coastal_noise(uint32_t noiseSeed, int coastBand, double noiseFreq)
{
    // Any ocean seed works as the owner to reassign eroded pixels to - we
    // only need seeds[...].type to read back as ocean for them afterwards.
    int oceanSeedIdx = -1;
    for (size_t i = 0; i < seeds.size(); ++i)
    {
        if (seeds[i].type == eBiome::ocean)
        {
            oceanSeedIdx = static_cast<int>(i);
            break;
        }
    }

    // Multi-source BFS distance-from-ocean transform, capped at coastBand,
    // so we only touch pixels near a shoreline instead of scanning the whole map.
    std::vector<std::vector<int>> dist(height, std::vector<int>(width, -1));
    std::queue<std::pair<int, int>> frontier;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (owner[y][x] >= 0 && seeds[owner[y][x]].type == eBiome::ocean)
            {
                dist[y][x] = 0;
                frontier.push({ x, y });
            }
        }
    }

    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dy[4] = { 0, 0, 1, -1 };

    while (!frontier.empty())
    {
        auto [x, y] = frontier.front();
        frontier.pop();

        int d = dist[y][x];
        if (d >= coastBand)
            continue;

        for (int i = 0; i < 4; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                continue;
            if (dist[ny][nx] != -1)
                continue;

            dist[ny][nx] = d + 1;
            frontier.push({ nx, ny });
        }
    }

    // Erode land pixels inside the coastal band using fbm noise, so the
    // shoreline gets ragged detail instead of voronoi's smooth cell edges.
    // Pixels closer to the true boundary are more likely to erode than
    // pixels near the far edge of the band.
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int d = dist[y][x];
            if (d <= 0 || d > coastBand)
                continue; // ocean itself, or too far inland to matter

            double n = (noise::fbm(x * noiseFreq, y * noiseFreq, noiseSeed, 4) + 1.0) * 0.5; // [0,1]
            double edge = static_cast<double>(d) / coastBand; // 0 at shore, 1 at band limit

            if (n > edge)
            {
                image[y][x] = color::AQUA;
                if (oceanSeedIdx >= 0)
                    owner[y][x] = oceanSeedIdx; // so downstream logic (e.g. junction detection) sees the real, eroded coastline
            }
        }
    }
}

std::vector<std::pair<int, int>> Voronoi::detect_land_land_ocean_junctions() const
{
    // For every ocean cell, look only at its 8 direct neighbours. If those
    // neighbours cover 2 or more distinct land biome types, this ocean cell
    // sits right where two different biomes meet the ocean - mark it.
    static const int dx8[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    static const int dy8[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

    std::vector<std::pair<int, int>> result;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (owner[y][x] < 0 || seeds[owner[y][x]].type != eBiome::ocean)
                continue; // only considering ocean cells

            std::unordered_set<eBiome> landTypes;

            for (int i = 0; i < 8; ++i)
            {
                int nx = x + dx8[i];
                int ny = y + dy8[i];

                if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                    continue;

                int o = owner[ny][nx];
                if (o < 0 || seeds[o].type == eBiome::ocean)
                    continue;

                landTypes.insert(seeds[o].type);
            }

            if (landTypes.size() >= 2)
                result.push_back({ x, y });
        }
    }

    return result;
}

void Voronoi::render_junction_markers(int markerRadius)
{
    junctions = detect_land_land_ocean_junctions();

    for (const auto& [jx, jy] : junctions)
    {
        for (int dy = -markerRadius; dy <= markerRadius; ++dy)
        {
            for (int dx = -markerRadius; dx <= markerRadius; ++dx)
            {
                if (dx * dx + dy * dy > markerRadius * markerRadius)
                    continue;

                int px = jx + dx;
                int py = jy + dy;

                if (px >= 0 && px < width && py >= 0 && py < height)
                    image[py][px] = color::RED;
            }
        }
    }
}

void Voronoi::render_seed_markers()
{
    for (const auto& seed : seeds)
    {
        int r = seed_marker_radius;
        for (int dx = -r; dx <= r; ++dx)
        {
            for (int dy = -r; dy <= r; ++dy)
            {
                if (dx * dx + dy * dy <= r * r)
                {
                    int px = seed.x + dx;
                    int py = seed.y + dy;
                    if (px >= 0 && px < width && py >= 0 && py < height)
                        image[py][px] = color::BLACK;
                }
            }
        }
    }
}

void Voronoi::generate(const std::vector<Seed>& biomeSeeds, int oceanSeedCount, double oceanMinDistFromBiome)
{
    srand(static_cast<unsigned>(time(nullptr)));

    seeds = biomeSeeds;
    generate_ocean_seeds(biomeSeeds, oceanSeedCount, oceanMinDistFromBiome);

    image.assign(height, std::vector<int>(width, color::BLACK));
    owner.assign(height, std::vector<int>(width, -1));

    render_voronoi_biomes();
    apply_coastal_noise((static_cast<uint32_t>(rand()) << 16) ^ static_cast<uint32_t>(rand()));
    render_seed_markers();
    render_junction_markers();
}