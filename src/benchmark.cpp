#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <thread>

#include "image.h"
#include "convolution.h"

using Clock = std::chrono::steady_clock;
constexpr int REPEATS = 5;

// Apskaičiuoja funkcijos vykdymo laiką milisekundėmis
template<typename F>
static double timeMs(F&& f) {
    auto t0 = Clock::now();
    f();
    auto t1 = Clock::now();
    
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

static double median(std::vector<double> v) {
    std::sort(v.begin(), v.end());
    std::size_t n = v.size();
    return (n % 2 == 0) ? (v[n / 2 - 1] + v[n / 2]) / 2.0 : v[n / 2];
}

int main() {
    try {
        const std::vector<int> sizes = {512, 1024, 2048, 4096};
        std::vector<int> threadCounts = {1, 2, 4, 6, 8, 10, 12, 16};
        int hw = static_cast<int>(std::thread::hardware_concurrency());
        if(hw > 0 && std::find(threadCounts.begin(), threadCounts.end(), hw) == threadCounts.end())
            threadCounts.push_back(hw);
        std::sort(threadCounts.begin(), threadCounts.end());

        std::filesystem::create_directories("results");
        std::ofstream csv("results/benchmark.csv");
        if(!csv) throw std::runtime_error("Nepavyko sukurti results/benchmark.csv");
        csv << "size,threads,seq_ms,par_ms,speedup,efficiency\n";

        std::cout << std::fixed << std::setprecision(3);
        for(int size : sizes) {
            Image input = generateTestImage(size, size);

            // 1 apšilimo bėgimas, tada REPEATS matavimų
            Image seqOut;
            timeMs([&] { seqOut = sobelSequential(input); });
            std::vector<double> seqTimes;
            for(int r=0; r < REPEATS; r++) {
                seqTimes.push_back(timeMs([&] { seqOut = sobelSequential(input); }));
            }
            double seqMs = median(seqTimes);

            for(int p : threadCounts) {
                Image parOut;
                timeMs([&] { parOut = sobelParallel(input, p); });  // apšilimas
                std::vector<double> parTimes;
                for (int r = 0; r < REPEATS; ++r)
                    parTimes.push_back(timeMs([&] { parOut = sobelParallel(input, p); }));
                
                if(parOut.data != seqOut.data)
                    throw std::runtime_error("Lygiagretus rezultatas nesutampa su nuosekliu");

                double parMs = median(parTimes);
                double speedup = seqMs / parMs;
                double efficiency = speedup / p;

                std::cout << size << "x" << size << "  p=" << std::setw(2) << p
                          << "  seq=" << seqMs << " ms  par=" << parMs << " ms  S=" << speedup
                          << "  E=" << efficiency << "\n";
                csv << size << "," << p << "," << seqMs << "," << parMs << ","
                    << speedup << "," << efficiency << "\n";
            }
        }
        std::cout << "Rezultatai išsaugoti results/benchmark.csv faile\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Klaida: " << e.what() << "\n";
        return 2;
    }
}