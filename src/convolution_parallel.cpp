#include "convolution.h"

#include <thread>
#include <algorithm>
#include <functional>
#include <vector>

Image sobelParallel(const Image& in, int numThreads, Order order) {
    Image out(in.width, in.height);

    // Dalijama pagal išorinio ciklo dimensiją: Row - eilutės, Column - stulpeliai
    const int extent = (order == Order::Row) ? in.height : in.width;
    numThreads = std::max(1, std::min(numThreads, extent));

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for(int t=0; t < numThreads; t++) {
        int begin = static_cast<int>(static_cast<long long>(extent) * t / numThreads);
        int end = static_cast<int>(static_cast<long long>(extent) * (t + 1) / numThreads);

        if(order == Order::Row)
            threads.emplace_back(sobelRegion, std::cref(in), std::ref(out), 0, in.width, begin, end, order);
        else
            threads.emplace_back(sobelRegion, std::cref(in), std::ref(out), begin, end, 0, in.height, order);
    }

    for(std::thread& t : threads) t.join();
    return out;
}
