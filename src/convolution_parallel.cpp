#include "convolution.h"

#include <thread>
#include <algorithm>
#include <functional>
#include <vector>

Image sobelParallel(const Image& in, int numThreads) {
    Image out(in.width, in.height);
    numThreads = std::max(1, std::min(numThreads, in.height));

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for(int t=0; t < numThreads; t++) {
        int rowBegin = static_cast<int>(static_cast<long long>(in.height) * t / numThreads);
        int rowEnd = static_cast<int>(static_cast<long long>(in.height) * (t + 1) / numThreads);
        threads.emplace_back(sobelRows, std::cref(in), std::ref(out), rowBegin, rowEnd);
    }

    for(std::thread& t : threads) t.join();
    return out;
}