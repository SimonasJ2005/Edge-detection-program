#include "convolution.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

static const int GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
static const int GY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

Order parseOrder(const std::string& name) {
    if(name == "row") return Order::Row;
    if(name == "col") return Order::Column;
    throw std::runtime_error("Nežinoma tvarka '" + name + "' (galima: row, col)");
}

const char* orderName(Order order) {
    return order == Order::Row ? "row" : "col";
}

// Vieno pikselio Sobel gradiento dydis
static inline std::uint8_t sobelPixel(const Image& in, int x, int y) {
    // Kraštiniai pikseliai neturi visų reikiamų kaimynų, todėl jų reikšmes paliekame 0
    if(x == 0 || y == 0 || x == in.width - 1 || y == in.height - 1) return 0;

    int gx = 0, gy = 0;
    for(int ky = -1; ky <= 1; ky++) {
        for(int kx = -1; kx <= 1; kx++) {
            int p = in.at(kx + x, ky + y);
            gx += GX[ky+1][kx+1] * p;
            gy += GY[ky+1][kx+1] * p;
        }
    }

    int mag = static_cast<int>(std::sqrt(static_cast<double>(gx * gx + gy * gy)));
    return static_cast<std::uint8_t>(std::min(255, mag));
}

void sobelRegion(const Image& in, Image& out, int x0, int x1, int y0, int y1, Order order) {
    if(order == Order::Row) {
        for(int y = y0; y < y1; y++)
            for(int x = x0; x < x1; x++)
                out.at(x, y) = sobelPixel(in, x, y);
    } else {
        for(int x = x0; x < x1; x++)
            for(int y = y0; y < y1; y++)
                out.at(x, y) = sobelPixel(in, x, y);
    }
}

Image sobelSequential(const Image& in, Order order) {
    Image out(in.width, in.height);
    sobelRegion(in, out, 0, in.width, 0, in.height, order);
    return out;
}
