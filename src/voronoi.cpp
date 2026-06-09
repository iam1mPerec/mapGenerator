#include <time.h>
#include "voronoi.hpp"
#include "colors.hpp"

Voronoi::Voronoi(int w, int h, int sc, int mr)
    : width(w), height(h), seed_count(sc), seed_marker_radius(mr)
{
    image.assign(height, std::vector<int>(width, color::BLACK));
    owner.assign(height, std::vector<int>(width, 0));
    seeds.resize(seed_count);
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
        seed.touches_border = false;
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

            if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
                seeds[best_seed].touches_border = true;
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
            if (seeds[seed_idx].touches_border)
            {
                image[y][x] = color::AQUA;
                seeds[seed_idx].type = eBiome::ocean;
            }
            else
            {
                seeds[seed_idx].type = getBiomeType(image[y][x]);
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

void Voronoi::generate()
{
    srand(static_cast<unsigned>(time(nullptr)));
    generate_random_seeds();
    render_voronoi();
    render_ocean();
    render_seed_markers();
}