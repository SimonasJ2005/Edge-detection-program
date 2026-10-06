#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> data;

    Image() = default;
    Image(int w, int h) : width(w), height(h), data(static_cast<std::size_t>(w) * h, 0) {}

    // Pikselis (x, y) yra data[y * width + x]
    std::uint8_t& at(int x, int y) { 
        return data[static_cast<std::size_t>(y) * width + x]; 
    }
    std::uint8_t at(int x, int y) const { 
        return data[static_cast<std::size_t>(y) * width + x]; 
    }
};

// Nuskaito PGM (P5) failą. Klaidos atveju meta std::runtime_error.
Image loadPGM(const std::string& path);

// Išsaugo vaizdą PGM (P5) formatu.
void savePGM(const Image& image, const std::string& path);

// Sugeneruoja testinį vaizdą su ryškiais kraštais (kvadratai, apskritimas, gradientas),
// kad nereikėtų atsisiųsti duomenų ir būtų galima pasirinkti bet kokį dydį.
Image generateTestImage(int width, int height);