#pragma once

#include "image.h"

// Apskaičiuoja Sobel rezultatą eilutėms [rowBegin, rowEnd). Lygiagreti versija
// kiekvienai gijai duos savo eilučių intervalą, todėl skaičiavimo kodas bendras.
void sobelRows(const Image& in, Image& out, int rowBegin, int rowEnd);

// Nuosekli versija
Image sobelSequential(const Image& in);

// Lygiagreti versija su std::thread
Image sobelParallel(const Image& in, int threads);