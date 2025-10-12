#include <QApplication>
#include <QPushButton>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <iostream>
#include "json.hpp"
#include "../datafields.h"

using json = nlohmann::json;
// --- Callbacks del fetch ---
void onSuccess(emscripten_fetch_t *fetch) {
    std::cout << "✅ Servidor respondió: "
              << std::string(fetch->data, fetch->numBytes) << std::endl;
    emscripten_fetch_close(fetch);
}

void onError(emscripten_fetch_t *fetch) {
    std::cout << "❌ Error: " << fetch->status << std::endl;
    emscripten_fetch_close(fetch);
}


std::string userToJson(user &reg)
{
    json j = {
        {User::name, reg.name},
        {User::lastName, reg.lastName},							
        {User::birthDate, reg.birthDate},
        {User::age, reg.age},
        {User::cellphone, reg.cellphone},
        {User::email, reg.email},
        {User::password, reg.password},
        {User::nroDoc, reg.nroDoc},
        {User::tipoDoc, reg.tipoDoc},
        {User::rol, reg.rol}
    };
    
    return j.dump();
}


// --- Función para enviar datos ---
void enviarDatos() {
    std::string nombre = "bartolomeo";
    std::string edad = "21";

    std::string jsonData = "{";
    jsonData += "\"nombre\":\"" + nombre + "\",";
    jsonData += "\"edad\":" + edad;
    jsonData += "}";

    std::cout << "📤 Enviando: " << jsonData << std::endl;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onSuccess;
    attr.onerror = onError;

    attr.requestData = jsonData.c_str();
    attr.requestDataSize = jsonData.length();

    const char* headers[] = {"Content-Type", "application/json", nullptr};
    attr.requestHeaders = headers;

    emscripten_fetch(&attr, "http://localhost:9080/data");
}

void enviarRegistro()
{
    user reg("omla@mail.com", "13");
    std::string jsonD = userToJson(reg);
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onSuccess;
    attr.onerror = onError;

    attr.requestData = jsonD.c_str();
    attr.requestDataSize = jsonD.length();

    const char* headers[] = {"Content-Type", "application/json", nullptr};
    attr.requestHeaders = headers;
    emscripten_fetch(&attr, "http://localhost:9080/register");
}

// --- Programa principal ---
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QPushButton boton("Enviar datos al servidor");
    boton.resize(220, 60);
    boton.show();

    QObject::connect(&boton, &QPushButton::clicked, []() {
        enviarRegistro();
    });

    return app.exec();
}
