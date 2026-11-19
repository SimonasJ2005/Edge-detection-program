#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <vector>

#include "image.h"
#include "convolution.h"

constexpr int REPEATS = 5;
static_assert(REPEATS % 2 == 1, "mediana - vidurinis elementas, todėl REPEATS turi būti nelyginis");

// Skaičiai rašomi su dešimtainiu kableliu
struct DecimalComma : std::numpunct<char> {
    char do_decimal_point() const override { return ','; }
};

// Paleidžia f vieną kartą apšilimui, tada REPEATS kartų; grąžina laikų medianą milisekundėmis
template<typename F>
static double medianMs(F f) {
    f(); // apšilimas
    std::vector<double> times;
    for(int r = 0; r < REPEATS; r++) {
        auto t0 = std::chrono::steady_clock::now();
        f();
        auto t1 = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[REPEATS / 2];
}

int main() {
    try {
        const std::vector<int> sizes = {512, 1024, 2048, 4096};
        const std::vector<Order> orders = {Order::Row, Order::Column};
        const std::vector<int> threadCounts = {1, 2, 4, 6, 8, 10, 12, 16};

        std::filesystem::create_directories("results");
        std::ofstream csv("results/benchmark.csv");
        if(!csv) throw std::runtime_error("Nepavyko sukurti results/benchmark.csv");
        csv.imbue(std::locale(csv.getloc(), new DecimalComma));
        csv << "size;order;threads;seq_ms;par_ms;speedup;efficiency\n";

        std::cout << std::fixed << std::setprecision(3);
        for(int size : sizes) {
            Image input = generateTestImage(size, size);
            // Visų versijų ir tvarkų rezultatai turi sutapti su šiuo
            const Image reference = sobelSequential(input, Order::Row);

            for(Order order : orders) {
                Image out;
                double seqMs = medianMs([&] { out = sobelSequential(input, order); });
                if(out.data != reference.data)
                    throw std::runtime_error("Nuoseklus rezultatas priklauso nuo prieigos tvarkos");

                for(int p : threadCounts) {
                    double parMs = medianMs([&] { out = sobelParallel(input, p, order); });
                    if(out.data != reference.data)
                        throw std::runtime_error("Lygiagretus rezultatas nesutampa su nuosekliu");

                    double speedup = seqMs / parMs;
                    double efficiency = speedup / p;

                    std::cout << size << "x" << size << "  " << orderName(order)
                              << "  p=" << std::setw(2) << p
                              << "  seq=" << seqMs << " ms  par=" << parMs << " ms  S=" << speedup
                              << "  E=" << efficiency << "\n";
                    csv << size << ";" << orderName(order) << ";" << p << ";" << seqMs << ";"
                        << parMs << ";" << speedup << ";" << efficiency << "\n";
                }
            }
        }
        std::cout << "Rezultatai issaugoti results/benchmark.csv faile\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Klaida: " << e.what() << "\n";
        return 2;
    }
}
