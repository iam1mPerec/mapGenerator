#include <iostream>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define SEEDS_COUNT 50

#define OUTPUT_FILE_PATH "output.ppm"

#define GROVE_BOX_COLOR_BRIGHT_GRAY		0xFF181818
#define GROVE_BOX_COLOR_BRIGHT_GREEN	0xFF26BBB8
#define GROVE_BOX_COLOR_BRIGHT_YELLOW	0xFF2FBDFA
#define GROVE_BOX_COLOR_BRIGHT_BLUE		0xFF98A583
#define GROVE_BOX_COLOR_BRIGHT_PURPLE	0xFF9B86D3
#define GROVE_BOX_COLOR_BRIGHT_AQUA		0xFFDCDC8D

#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xFFFFFFFF
#define SEED_MARKER_RADIUS 5

typedef int Color32;

enum eBiome {
	ocean,
	desert,
	swamp,
	forest,
	rock,
	ruin
};

struct Seed {
	int x, y = 0;
	bool touches_border = false;
	eBiome type = ocean;
};

static Color32 image[HEIGHT][WIDTH];
static Seed seeds[SEEDS_COUNT];
static int owner[HEIGHT][WIDTH];
static Color32 palette[] = {
	GROVE_BOX_COLOR_BRIGHT_GRAY,
	GROVE_BOX_COLOR_BRIGHT_GREEN,
	GROVE_BOX_COLOR_BRIGHT_YELLOW,
	GROVE_BOX_COLOR_BRIGHT_PURPLE,
	GROVE_BOX_COLOR_BRIGHT_BLUE
};
#define palette_count (sizeof(palette)/sizeof(palette[0]))

void fill_image(Color32 color) {
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			image[y][x] = color;
		}
	}
}

void save_image_as_ppm(const char* file_path) 
{
	FILE* f = nullptr;
	fopen_s(&f, file_path, "wb");
	if (f == NULL) {
		return;
	}
	else {
		fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT);
	}
	for (size_t y = 0; y < HEIGHT; ++y)
	{
		for (size_t x = 0; x < WIDTH; ++x) {
			uint32_t pixel = image[y][x];
			uint8_t bytes[3] = {
				(pixel & 0x0000FF) >> 8 * 0,
				(pixel & 0x00FF00) >> 8 * 1,
				(pixel & 0xFF0000) >> 8 * 2,
			};
			fwrite(bytes, sizeof(bytes), 1, f);
			assert(!ferror(f));
		}
	}
	int ret = fclose(f);
	assert(ret == 0);
}

void generate_random_seeds() {
	for (size_t i = 0; i < SEEDS_COUNT; i++)
	{
		seeds[i].x = rand() % WIDTH;
		seeds[i].y = rand() % HEIGHT;
	}
}

int sqr_dist(int x1, int y1, int x2, int y2) {
	int dx = x1 - x2;
	int dy = y1 - y2;
	return dx * dx + dy * dy;
}

void fill_circle(int cx, int cy, int radius, Color32 color) {
	int x0 = cx - radius;
	int y0 = cy - radius;
	int x1 = cx + radius;
	int y1 = cy + radius;

	for (int x = x0; x <= x1; ++x) {
		if (0 <= x && x < WIDTH)
		{
			for (int y = y0; y <= y1; ++y) {
				if (0 <= y && y < HEIGHT) {
					if (sqr_dist(cx, cy, x, y) <= radius * radius) {
						image[y][x] = color;
					}
				}
				
			}
		}
	}
}

void render_seed_markers() {
	for (size_t i = 0; i < SEEDS_COUNT; ++i) {
		fill_circle(seeds[i].x, seeds[i].y, SEED_MARKER_RADIUS, COLOR_BLACK);
	}
}

void render_voronoi() {
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {

			int best_seed = 0;

			int best_dist = sqr_dist(
				seeds[0].x,
				seeds[0].y,
				x, y
			);

			for (int i = 1; i < SEEDS_COUNT; ++i) {

				int d = sqr_dist(
					seeds[i].x,
					seeds[i].y,
					x, y
				);

				if (d < best_dist) {
					best_dist = d;
					best_seed = i;
				}
			}

			owner[y][x] = best_seed;
			image[y][x] = palette[best_seed % palette_count];

			// detect border-touching seeds
			if (x == 0 || x == WIDTH - 1 ||
				y == 0 || y == HEIGHT - 1) {

				seeds[best_seed].touches_border = true;
			}
		}
	}
}

eBiome getBiomeType(Color32 color) {
	if (color == GROVE_BOX_COLOR_BRIGHT_GRAY) return eBiome::ruin;
	if (color == GROVE_BOX_COLOR_BRIGHT_GREEN) return eBiome::forest;
	if (color == GROVE_BOX_COLOR_BRIGHT_YELLOW) return eBiome::desert;
	if (color == GROVE_BOX_COLOR_BRIGHT_PURPLE) return eBiome::swamp;
	if (color == GROVE_BOX_COLOR_BRIGHT_BLUE) return eBiome::rock;
}

void render_ocean() {
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {

			int seed = owner[y][x];

			if (seeds[seed].touches_border) {
				image[y][x] = GROVE_BOX_COLOR_BRIGHT_AQUA;
				seeds[seed].type = eBiome::ocean;
			}
			else {
				seeds[seed].type = getBiomeType(image[y][x]);
			}
		}
	}
}

int main() {
	srand(time(NULL));
	generate_random_seeds();
	render_voronoi();
	render_ocean();
	render_seed_markers();
	save_image_as_ppm((OUTPUT_FILE_PATH));
}