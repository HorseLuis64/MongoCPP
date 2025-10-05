#include <emscripten.h>
#include <emscripten/fetch.h>   // mejor usar fetch en vez de wget2 (es más moderno)
#include <iostream>
#include <string>
#include "json.hpp"

// callback cuando se recibe toda la respuesta
void onSuccess(emscripten_fetch_t *fetch) {
    std::string response(fetch->data, fetch->numBytes);
    std::cout << "Respuesta del servidor:\n" <<response << std::endl;

    try {
        auto j = nlohmann::json::parse(response);
        std::cout << "Campo name: " << j["name"] << "\n";
        std::cout << "Campo age: " << j["age"] << "\n";
    } catch (std::exception &e) {
        std::cout << "Error parseando JSON: " << e.what() << std::endl;
    }

    emscripten_fetch_close(fetch);
}

// callback de error
void onError(emscripten_fetch_t *fetch) {
    std::cout << "Error HTTP (" << fetch->status << "): " 
              << (fetch->statusText ? fetch->statusText : "sin mensaje") 
              << std::endl;
    emscripten_fetch_close(fetch);
}

// callback de progreso
void onProgress(emscripten_fetch_t *fetch) {
    if (fetch->totalBytes > 0) {
        double pct = (double)fetch->dataOffset / (double)fetch->totalBytes * 100.0;
        std::cout << "Progreso: " << pct << "%\n";
    }
}

int main() {
    std::cout << "Haciendo request a API REST desde C++/WASM...\n";

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");

    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess  = onSuccess;
    attr.onerror    = onError;
    attr.onprogress = onProgress;

    emscripten_fetch(&attr, "http://localhost:9080/data");

    return 0;
}
