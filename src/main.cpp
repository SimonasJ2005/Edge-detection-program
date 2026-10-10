#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include <stdexcept>
#include <filesystem>

#include "image.h"
#include "convolution.h"

using Clock = std::chrono::steady_clock;

struct Options {
    int size = 1024;
    int threads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    std::string input; // Jei tuščias - generuojamas size*size failas
};

// Skaito poras --argumentoVardas argumentoReikšmė, neprivaloma įvesti visus argumentus
static Options parseArgs(int argc, char** argv) {
    Options opt;

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if(i + 1 >= argc) throw std::runtime_error("Trūksta reikšmės parametrui " + arg);

        std::string value = argv[i++];
        if(arg == "--size") opt.size = std::stoi(value);
        else if(arg == "--threads") opt.threads = std::stoi(value);
        else if(arg == "--input") opt.input = value;
        else throw std::runtime_error("Nežinomas parametras " + arg);
    }
    if(opt.size < 3) throw std::runtime_error("--size turi būti bent 3");
    if(opt.threads < 1) throw std::runtime_error("--threads turi būti bent 1");
    return opt;
}

int main(int argc, char** argv) {
    try {
        Options opt = parseArgs(argc, argv);

        Image input = opt.input.empty() ? generateTestImage(opt.size, opt.size)
                                        : loadImage(opt.input);
        std::cout << "Vaizdas: " << input.width << "x" << input.height << ", gijos = " << opt.threads << "\n";
        
        auto t0 = Clock::now();
        Image seq = sobelSequential(input);
        auto t1 = Clock::now();
        Image par = sobelParallel(input, opt.threads);
        auto t2 = Clock::now();

        double seqMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double parMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
        bool same = seq.data == par.data; // Ar vaizdai identiški  

        std::cout << "Nuosekli:    " << seqMs << " ms\n"
                  << "Lygiagreti:  " << parMs << " ms\n"
                  << "Pagreitejimas: " << seqMs / parMs << "x\n"
                  << "Rezultatai " << (same ? "SUTAMPA" : "NESUTAMPA") << "\n";

        std::filesystem::create_directories("img");
        savePGM(input, "img/input.pgm");
        savePGM(par, "img/edges.pgm");
        saveImage(input, "img/input.png");
        saveImage(par, "img/edges.png");
        return same ? 0 : 1;
    } catch(const std::exception& e) {
        std::cerr << "Klaida: " << e.what() << "\n";
        return 2;
    }
}