#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "nodesoup/nodesoup.hpp"

// Turns a nodesoup graph into a biome ppm map.
//
// Splitting the work into generateLayout()/buildMap() lets the expensive,
// graph-topology-only Kamada-Kawai layout be computed once and reused: it is
// fully deterministic for a given graph + width/height/margin/k/energyThreshold,
// so it only needs to be redone when the graph shape or those layout settings
// change, not every time biome assignment or rendering is re-run.
class MapRenderer
{
public:
    MapRenderer() = default;

    MapRenderer& setWidth(int width);
    MapRenderer& setHeight(int height);
    MapRenderer& setLayoutMargin(int margin);
    MapRenderer& setBiomeInfluenceRadius(double radius);
    MapRenderer& setOceanSeedCount(int count);
    MapRenderer& setSeedMarkerRadius(int radius);
    MapRenderer& setOutputFilePath(const std::string& path);
    MapRenderer& setLayoutK(double k);
    MapRenderer& setLayoutEnergyThreshold(double threshold);

    // Debug overlays: draw the underlying graph's nodes (as biome-colored circles)
    // and/or edges (as black lines) on top of the rendered biome map, so the
    // generation can be visually inspected during development.
    MapRenderer& setRenderPoints(bool enabled);
    MapRenderer& setRenderLines(bool enabled);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getLayoutMargin() const { return layoutMargin; }

    // Computed from width/height/layoutMargin - no setter, kept in sync automatically.
    int getLayoutWidth() const { return width - 2 * layoutMargin; }
    int getLayoutHeight() const { return height - 2 * layoutMargin; }

    // Step 1 (slow): lays out the graph and caches the result to layoutFilePath.
    void generateLayout(const nodesoup::adj_list_t& graph, const std::string& layoutFilePath) const;

    // Step 2 (fast): loads a layout produced by generateLayout() and renders the
    // biome map (voronoi + coastline carving) to outputFilePath.
    void buildMap(const nodesoup::adj_list_t& graph, const std::string& layoutFilePath) const;

    // Convenience one-shot: computes the layout and renders the map without caching it.
    void generate(const nodesoup::adj_list_t& graph) const;

private:
    int width = 1600;
    int height = 1200;

    // Empty border kept around the node layout. The graph is laid out inside
    // (width - 2*layoutMargin) x (height - 2*layoutMargin) but still centered on
    // the full canvas, so voronoi has room to grow biomes past the outermost nodes.
    int layoutMargin = 125;

    double biomeInfluenceRadius = 75.0;
    int oceanSeedCount = 30;
    int seedMarkerRadius = 5;
    double layoutK = 200.0;
    double layoutEnergyThreshold = 1.0;
    std::string outputFilePath = "output.ppm";
    bool renderPoints = false;
    bool renderLines = false;

    struct Layout
    {
        std::vector<nodesoup::Point2D> positions;
        std::vector<double> radiuses;
    };

    Layout computeLayout(const nodesoup::adj_list_t& graph) const;
    void saveLayout(const std::string& path, const Layout& layout) const;
    Layout loadLayout(const std::string& path, size_t expectedVertexCount) const;
    void renderToPpm(const nodesoup::adj_list_t& graph, Layout layout, const std::string& filename) const;

    static void save_image_as_ppm(
        const std::string& filePath,
        int width,
        int height,
        const std::vector<std::vector<int>>& image);

    static void draw_line(
        std::vector<std::vector<int>>& image,
        int x0, int y0, int x1, int y1,
        int width, int height,
        uint32_t color);

    static void draw_circle(
        std::vector<std::vector<int>>& image,
        int cx, int cy, int radius,
        int width, int height,
        uint32_t color);
};
