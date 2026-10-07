#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "image.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION

// Praleidžia tarpus ir komentarus (eilutes, prasidedančias '#') PGM antraštėje.
static void skipCommentsAndWhiteSpaces(std::istream& in) {
    while(true) {
        int c = in.peek();

        if(c == '#') {
            std::string line;
            std::getline(in, line);
        }
        else if(std::isspace(c)) {
            in.get();
        }
        else {
            break;
        }
    }
}

Image loadPGM(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if(!in) throw std::runtime_error("Nepavyko atidaryti failo: " + path);

    std::string magic;
    in >> magic;
    if(magic != "P5") throw std::runtime_error("Palaikomas tik PGM P5 formatas: " + path);

    int w = 0, h = 0, maxVal = 0;
    skipCommentsAndWhiteSpaces(in);
    in >> w;
    skipCommentsAndWhiteSpaces(in);
    in >> h;
    skipCommentsAndWhiteSpaces(in);
    in >> maxVal;
    in.get();

    if(!in || w <= 0 || h <= 0 || maxVal != 255) throw std::runtime_error("Neteisinga PGM antraštė: " + path);

    Image img(w, h);
    in.read(reinterpret_cast<char*>(img.data.data()), static_cast<std::streamsize>(img.data.size()));
    if(!in) throw std::runtime_error("Failo pabaiga per anksti: " + path);

    return img;
}

void savePGM(const Image& image, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if(!out) throw std::runtime_error("Nepavyko sukurti failo: " + path);

    out << "P5\n" << image.width << " " << image.height << "\n255\n";
    out.write(reinterpret_cast<const char*>(image.data.data()),
              static_cast<std::streamsize>(image.data.size()));

    if(!out) throw std::runtime_error("Nepavyko įrašyti failo: " + path);
}

Image loadImage(const std::string& path) {
    int w = 0, h = 0, channels = 0;

    // Atidaro failą, jį dekoduoja ir gražina pikselių masyvą atmintyje. Taip pat grąžina w ir h.
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &channels, 1);
    if(!pixels) throw std::runtime_error("Nepavyko nuskaityti vaizdo: " + path);

    Image img(w, h);
    std::copy(pixels, pixels + static_cast<std::size_t>(w) * h, img.data.begin());
    stbi_image_free(pixels);

    return img;
}

void saveImage(const Image& img, const std::string& path) {
    int dot = path.find_last_of('.');
    std::string extension = (dot == std::string::npos) ? "" : path.substr(dot + 1);
    for(char& c : extension) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

    int ok = 0;
    if(extension == "png")
        ok = stbi_write_png(path.c_str(), img.width, img.height, 1, img.data.data(), img.width);
    else if (extension == "png" || extension == "jpeg")
        ok = stbi_write_png(path.c_str(), img.width, img.height, 1, img.data.data(), 95);
    else if (extension == "pgm") {
        savePGM(img, path);
        return;
    }
    else throw std::runtime_error("Nepalaikomas išvesties formatas: " + path);

    if(!ok) throw std::runtime_error("Nepavyko išsaugoti vaizdo: " + path);
}

Image generateTestImage(int width, int height) {
    Image img(width, height);

    const double cx = width * 0.7, cy = height * 0.3;
    const double radius = std::min(width, height) * 0.15;

    for (int y = 0; y < height; ++y) {          // eilutės: einame atmintimi iš eilės
        for (int x = 0; x < width; ++x) {
            // Fonas: švelnus gradientas iš kairės į dešinę (0..120)
            int value = static_cast<int>(120.0 * x / width);

            // Du ryškūs stačiakampiai
            bool rect1 = x > width / 10 && x < width * 3 / 10 && y > height / 10 && y < height * 4 / 10;
            bool rect2 = x > width / 5 && x < width * 6 / 10 && y > height * 6 / 10 && y < height * 9 / 10;
            if (rect1) value = 255;
            if (rect2) value = 30;

            // Apskritimas
            double dx = x - cx, dy = y - cy;
            if (dx * dx + dy * dy < radius * radius) value = 220;

            img.at(x, y) = static_cast<std::uint8_t>(value);
        }
    }
    return img;
}