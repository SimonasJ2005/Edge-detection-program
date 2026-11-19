#pragma once

#include <string>

#include "image.h"

// Atminties prieigos tvarka, kuria einama per vaizdą:
//  Row    - išorinis ciklas per eilutes (y), vidinis per stulpelius (x). Pikseliai
//           skaitomi iš eilės, kaip jie išdėstyti atmintyje.
//  Column - išorinis ciklas per stulpelius (x), vidinis per eilutes (y). Kiekvienas
//           žingsnis peršoka per visą eilutę (width baitų), todėl cache išnaudojamas prastai.
enum class Order { Row, Column };

// "row" -> Order::Row, "col" -> Order::Column. Kitu atveju meta std::runtime_error.
Order parseOrder(const std::string& name);

// Order::Row -> "row", Order::Column -> "col"
const char* orderName(Order order);

// Apskaičiuoja Sobel rezultatą stačiakampiui [x0, x1) × [y0, y1), eidama nurodyta tvarka.
// Lygiagreti versija kiekvienai gijai duos savo sritį, todėl skaičiavimo kodas bendras.
void sobelRegion(const Image& in, Image& out, int x0, int x1, int y0, int y1, Order order);

// Nuosekli versija
Image sobelSequential(const Image& in, Order order);

// Lygiagreti versija su std::thread. Vaizdas dalijamas pagal išorinio ciklo dimensiją:
// Row tvarka - eilučių juostomis, Column tvarka - stulpelių juostomis. Taip kiekviena
// gija savo srityje eina per atmintį taip pat, kaip ir nuosekli versija.
Image sobelParallel(const Image& in, int threads, Order order);
