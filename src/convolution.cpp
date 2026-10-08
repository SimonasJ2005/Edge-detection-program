#include "convolution.h"

#include <algorithm>
#include <cmath>

static const int GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
static const int GY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

void sobelRows(const Image& in, Image& out, int rowBegin, int rowEnd) {
    const int width = in.width;
    const int height = in.height;

    for(int y = rowBegin; y < rowEnd; y++) {
        for(int x = 0; x < width; x++) {
            // Kraštiniai pixeliai neturi visų reikiamų kaimynų, todėl jų reikšmes paliekame 0
            if(x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                out.at(x, y) = 0;
                continue;
            }

            int gx = 0, gy = 0;
            for(int ky = -1; ky <= 1; ky++) {
                for(int kx = -1; kx <= 1; kx++) {
                    int p = in.at(kx + x, ky + y);
                    gx += GX[ky+1][kx+1] * p;
                    gx += GY[ky+1][kx+1] * p;
                }
            }

            int mag = static_cast<int>(std::sqrt(static_cast<double>(gx * gx + gy * gy)));
            out.at(x, y) = std::min(255, mag);
        }
    }
}

Image sobelSequential(const Image& img) {
    Image out(img.width, img.height);
    sobelRows(img, out, 0, img.height);
    return out;
}