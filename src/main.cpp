#include <iostream>
#include <cassert>
#include "voronoi.hpp"
#include "node.hpp"
#include "nodesoup/nodesoup.hpp"
#include <algorithm>
#include <chrono>
#include <vector>
#include <unordered_set>


using namespace nodesoup;

using std::string;
using std::vector;
using std::cout;
using std::cerr;
std::unordered_set<vertex_id_t> marked;

constexpr auto WIDTH = 1600;
constexpr auto HEIGHT = 1200;
constexpr auto SEEDS_COUNT = 50;

constexpr auto OUTPUT_FILE_PATH = "output.ppm";
constexpr auto SEED_MARKER_RADIUS = 5;

static void save_image_as_ppm(
	const char* file_path,
	int width,
	int height,
	const std::vector<std::vector<int>>& image)
{
	FILE* f = nullptr;
	fopen_s(&f, file_path, "wb");

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

static void draw_line(vector<vector<int>>& image, int x0, int y0, int x1, int y1, int width, int height, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            image[y0][x0] = color;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) err -= dy, x0 += sx;
        if (e2 < dx) err += dx, y0 += sy;
    }
}

static void draw_circle(vector<vector<int>>& image, int cx, int cy, int radius, int width, int height, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                int px = cx + x;
                int py = cy + y;

                if (px >= 0 && px < width && py >= 0 && py < height) {
                    image[py][px] = color;
                }
            }
        }
    }
}

static void write_to_ppm(adj_list_t& g, vector<Point2D>& positions, vector<double>& radiuses, unsigned int width, unsigned int height, const char* filename) {
    // Create image buffer with packed RGB values (0xBBGGRR format)
    vector<vector<int>> image(height, vector<int>(width, 0xFFFFFF)); // white background

    // shift origin to center
    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        positions[v_id].x += width / 2.0;
        positions[v_id].y += height / 2.0;
    }

    // Draw edges (black = 0x000000)
    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        Point2D v_pos = positions[v_id];

        for (auto adj_id : g[v_id]) {
            if (adj_id < v_id) continue;

            Point2D adj_pos = positions[adj_id];
            draw_line(image, (int)v_pos.x, (int)v_pos.y, (int)adj_pos.x, (int)adj_pos.y, width, height, 0x000000);
        }
    }

    // Draw vertices (black = 0x000000)
    for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
        Point2D v_pos = positions[v_id];
        uint32_t color = marked.contains(v_id) ? 0x0000FF : 0x000000;
        draw_circle(image, (int)v_pos.x, (int)v_pos.y, (int)radiuses[v_id], width, height, color);
    }

    save_image_as_ppm(filename, width, height, image);
}

static void adj_list_to_ppm(
    adj_list_t& g,
    const char* ppm_filename,
    unsigned int width,
    unsigned int height,
    double k,
    double energy_threshold) {

    vector<Point2D> positions;
    vector<double> radiuses = size_radiuses(g);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    cout << "Laying out graph with Kamada-Kawai...\n";

    if (k == -1.0) {
        k = 300.0;
    }

    start = std::chrono::system_clock::now();
    positions = kamada_kawai(g, width, height, k, energy_threshold);
    end = std::chrono::system_clock::now();

    write_to_ppm(g, positions, radiuses, width, height, ppm_filename);

    unsigned int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Layout computed in " << ms << "ms\n";
}

int main() {
    node root("A", 0);
    root["C"].bulkPopulateRelative(0, 4);
    root["D"].bulkPopulateRelative(0, 4);
    root["E"].bulkPopulateRelative(0, 4);

    root["B"]["F"]["F.1"].bulkPopulateRelative(0, 4);
    root["B"]["F"]["F.1"]["F.1.2"].bulkPopulateRelative(0, 4);
	root["B"]["F"]["F.2"]["F.2.2"].bulkPopulateRelative(0, 4);
    root["B"]["F"]["F.2"]["F.2.3"].bulkPopulateRelative(0, 4);

    root["B"]["G"]["G.1"]["G.1.2"].bulkPopulateRelative(0, 4);
    root["B"]["G"]["G.1"]["G.1.1"].bulkPopulateRelative(0, 4);
    root["B"]["G"]["G.2"].bulkPopulateRelative(0, 4);

    root["C"]["C1"].bulkPopulateRelative(0, 4);
    root["D"]["D1"].bulkPopulateRelative(0, 4);
    root["E"]["E1"].bulkPopulateRelative(0, 4);
    auto graph = root.buildGraph();

    marked.insert(root["B"]["G"]["G.2"].getPosition());
    marked.insert(root["B"]["G"].getPosition());
    marked.insert(root["B"].getPosition());

     adj_list_to_ppm(graph, "output.ppm", WIDTH, HEIGHT, 200.0, 1);

	//Voronoi voronoi(WIDTH, HEIGHT, SEEDS_COUNT, SEED_MARKER_RADIUS);
	//voronoi.generate();
	//save_image_as_ppm(OUTPUT_FILE_PATH, voronoi.getWidth(), voronoi.getHeight(), voronoi.getImage());
}