#pragma once

namespace color {
    constexpr uint32_t GRAY = 0xFF181818;
    constexpr uint32_t GREEN = 0xFF26BBB8;
    constexpr uint32_t YELLOW = 0xFF2FBDFA;
    constexpr uint32_t BLUE = 0xFF98A583;
    constexpr uint32_t PURPLE = 0xFF9B86D3;
    constexpr uint32_t AQUA = 0xFFDCDC8D;
    constexpr uint32_t BLACK = 0xFF000000;
    constexpr uint32_t COLOR_WHITE = 0xFFFFFFFF;
}

typedef int Color32;

static Color32 palette[] = {
	color::GRAY,
    color::GREEN,
	color::PURPLE,
	color::YELLOW,
	color::BLUE
};

#define palette_count (sizeof(palette)/sizeof(palette[0]))