#include "mapRenderer.hpp"

#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#include "node.hpp"
#include "voronoi.hpp"

using namespace nodesoup;
using std::string;
using std::vector;

MapRenderer& MapRenderer::setWidth(int w) { width = w; return *this; }
MapRenderer& MapRenderer::setHeight(int h) { height = h; return *this; }
MapRenderer& MapRenderer::setLayoutMargin(int margin) { layoutMargin = margin; return *this; }
MapRenderer& MapRenderer::setBiomeInfluenceRadius(double radius) { biomeInfluenceRadius = radius; return *this; }
MapRenderer& MapRenderer::setOceanSeedCount(int count) { oceanSeedCount = count; return *this; }
MapRenderer& MapRenderer::setSeedMarkerRadius(int radius) { seedMarkerRadius = radius; return *this; }
MapRenderer& MapRenderer::setOutputFilePath(const string& path) { outputFilePath = path; return *this; }
MapRenderer& MapRenderer::setLayoutK(double k) { layoutK = k; return *this; }
MapRenderer& MapRenderer::setLayoutEnergyThreshold(double threshold) { layoutEnergyThreshold = threshold; return *this; }
MapRenderer& MapRenderer::setRenderPoints(bool enabled) { renderPoints = enabled; return *this; }
MapRenderer& MapRenderer::setRenderLines(bool enabled) { renderLines = enabled; return *this; }

void MapRenderer::save_image_as_ppm(
    const string& filePath,
    int width,
    int height,
    const vector<vector<int>>& image)
{
    FILE* f = nullptr;
    fopen_s(&f, filePath.c_str(), "wb");

    if (!f)
        return;

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint32_t pixel = static_cast<uint32_t>(image[y][x]);

            uint8_t r = (pixel >> 0) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;

            uint8_t bytes[3] = { r, g, b };

            fwrite(bytes, 1, 3, f);
            assert(!ferror(f));
        }
    }

    fclose(f);
}

void MapRenderer::draw_line(vector<vector<int>>& image, int x0, int y0, int x1, int y1, int width, int height, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
            image[y0][x0] = color;

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) err -= dy, x0 += sx;
        if (e2 < dx) err += dx, y0 += sy;
    }
}

void MapRenderer::draw_circle(vector<vector<int>>& image, int cx, int cy, int radius, int width, int height, uint32_t color)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                int px = cx + x;
                int py = cy + y;

                if (px >= 0 && px < width && py >= 0 && py < height)
                    image[py][px] = color;
            }
        }
    }
}

MapRenderer::Layout MapRenderer::computeLayout(const adj_list_t& graph) const
{
    Layout layout;
    layout.radiuses = size_radiuses(graph);

    std::cout << "Laying out graph with Kamada-Kawai...\n";
    auto start = std::chrono::system_clock::now();
    layout.positions = kamada_kawai(graph, getLayoutWidth(), getLayoutHeight(), layoutK, layoutEnergyThreshold);
    auto end = std::chrono::system_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Layout computed in " << ms << "ms\n";

    return layout;
}

void MapRenderer::saveLayout(const string& path, const Layout& layout) const
{
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "wb");

    if (!f)
        throw std::runtime_error("MapRenderer: failed to open layout file for writing: " + path);

    uint64_t vertexCount = layout.positions.size();
    fwrite(&vertexCount, sizeof(vertexCount), 1, f);
    fwrite(layout.positions.data(), sizeof(Point2D), layout.positions.size(), f);
    fwrite(layout.radiuses.data(), sizeof(double), layout.radiuses.size(), f);

    fclose(f);
}

MapRenderer::Layout MapRenderer::loadLayout(const string& path, size_t expectedVertexCount) const
{
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "rb");

    if (!f)
        throw std::runtime_error("MapRenderer: failed to open layout file for reading: " + path);

    uint64_t vertexCount = 0;
    fread(&vertexCount, sizeof(vertexCount), 1, f);

    if (vertexCount != expectedVertexCount)
    {
        fclose(f);
        throw std::runtime_error("MapRenderer: cached layout at " + path +
            " does not match graph size (cached graph likely changed) - regenerate it with generateLayout()");
    }

    Layout layout;
    layout.positions.resize(vertexCount);
    layout.radiuses.resize(vertexCount);

    fread(layout.positions.data(), sizeof(Point2D), vertexCount, f);
    fread(layout.radiuses.data(), sizeof(double), vertexCount, f);

    fclose(f);
    return layout;
}

void MapRenderer::renderToPpm(const adj_list_t& graph, Layout layout, const string& filename) const
{
    // shift origin to center
    for (vertex_id_t v_id = 0; v_id < graph.size(); v_id++)
    {
        layout.positions[v_id].x += width / 2.0;
        layout.positions[v_id].y += height / 2.0;
    }

    vector<Voronoi::Seed> biomeSeeds;
    biomeSeeds.reserve(graph.size());
    for (vertex_id_t v_id = 0; v_id < graph.size(); v_id++)
    {
        node* n = node::getNodeByPosition(static_cast<int32_t>(v_id));
        Voronoi::Seed s;
        s.x = static_cast<int>(layout.positions[v_id].x);
        s.y = static_cast<int>(layout.positions[v_id].y);
        s.type = n ? n->getBiome() : eBiome::ocean;
        s.influenceRadius = biomeInfluenceRadius;
        s.hasChildren = n ? n->hasChildren() : false;
        biomeSeeds.push_back(s);
    }

    Voronoi voronoi(width, height, seedMarkerRadius);
    voronoi.generate(biomeSeeds, oceanSeedCount, biomeInfluenceRadius);

    vector<vector<int>> image = voronoi.getImage();

    if (renderLines)
    {
        for (vertex_id_t v_id = 0; v_id < graph.size(); v_id++)
        {
            for (auto adj_id : graph[v_id])
            {
                if (adj_id < v_id) continue;

                draw_line(image,
                    static_cast<int>(layout.positions[v_id].x), static_cast<int>(layout.positions[v_id].y),
                    static_cast<int>(layout.positions[adj_id].x), static_cast<int>(layout.positions[adj_id].y),
                    width, height, 0x000000);
            }
        }
    }

    if (renderPoints)
    {
        // Marked from the voronoi's own seed list (graph nodes + the extra ocean
        // filler seeds it generates) rather than the biome's own color, since a
        // node's circle drawn in its own biome color is invisible against the
        // same-colored cell it seeded.
        for (const auto& seed : voronoi.getSeeds())
        {
            draw_circle(image, seed.x, seed.y, seedMarkerRadius, width, height, 0x000000);
        }
    }

    save_image_as_ppm(filename, width, height, image);
}

void MapRenderer::generateLayout(const adj_list_t& graph, const string& layoutFilePath) const
{
    Layout layout = computeLayout(graph);
    saveLayout(layoutFilePath, layout);
}

void MapRenderer::buildMap(const adj_list_t& graph, const string& layoutFilePath) const
{
    Layout layout = loadLayout(layoutFilePath, graph.size());
    renderToPpm(graph, std::move(layout), outputFilePath);
}

void MapRenderer::generate(const adj_list_t& graph) const
{
    Layout layout = computeLayout(graph);
    renderToPpm(graph, std::move(layout), outputFilePath);
}
