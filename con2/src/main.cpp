#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <iostream>
#include "json.hpp"   
#include <fstream>
#include "db_handler.h"

using json = nlohmann::json;
using namespace Pistache;

class RestApi {
public:
    RestApi(Address addr, std::shared_ptr<hdb::MongoHandler> dbHandler)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)), handler(dbHandler) {}

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(thr))
                        .flags(Tcp::Options::ReuseAddr); // Corregido: sin Http::
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;

        // IMPORTANTE: Primero definir OPTIONS para todas las rutas
        Routes::Options(router, "/data", Routes::bind(&RestApi::handleOptions, this));
        Routes::Options(router, "/data/:field/:value", Routes::bind(&RestApi::handleOptions, this));
        Routes::Options(router, "/*", Routes::bind(&RestApi::handleOptions, this));

        // Rutas de la API
        Routes::Get(router, "/data", Routes::bind(&RestApi::getAllData, this));
        Routes::Get(router, "/data/:field/:value", Routes::bind(&RestApi::getOneData, this));
        Routes::Post(router, "/data", Routes::bind(&RestApi::insertData, this));
        Routes::Put(router, "/data/:field/:value", Routes::bind(&RestApi::updateData, this));
        Routes::Delete(router, "/data/:field/:value", Routes::bind(&RestApi::deleteData, this));

        // Ruta estática para frontend - DEBE IR AL FINAL
        Routes::Get(router, "/*", Routes::bind(&RestApi::serveStatic, this));
    }

    void serveStatic(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        
        auto path = req.resource();
        if (path == "/" || path.empty()) {
            path = "/main.html";
        }

        // Si es una ruta de API, no servir como estático
        if (path.find("/data") == 0) {
            resp.send(Http::Code::Not_Found, "Ruta de API no encontrada");
            return;
        }

        std::string fullPath = "../weboo" + path;

        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open()) {
            resp.send(Http::Code::Not_Found, "Archivo no encontrado: " + fullPath);
            return;
        }

        std::string mime = "text/plain";
        if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") mime = "text/html";
        else if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") mime = "application/javascript";
        else if (path.size() >= 5 && path.substr(path.size() - 5) == ".wasm") mime = "application/wasm";
        else if (path.size() >= 5 && path.substr(path.size() - 5) == ".json") mime = "application/json";

        std::ostringstream ss;
        ss << file.rdbuf();

        resp.headers().add<Http::Header::ContentType>(mime);
        resp.send(Http::Code::Ok, ss.str());
    }

    // GET /data
    void getAllData(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        
        try {
            mongocxx::cursor cursor = handler->defaultCursor();
            handler->GetAllData(cursor);

            json arr = json::array();
            for (auto&& doc : cursor) {
                arr.push_back(json::parse(bsoncxx::to_json(doc)));
            }
            resp.send(Http::Code::Ok, arr.dump());
        } catch (const std::exception& e) {
            resp.send(Http::Code::Internal_Server_Error, "Error del servidor");
        }
    }

    // GET /data/:field/:value
    void getOneData(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        auto result = handler->GetOneData(field, value);
        if (result) {
            resp.send(Http::Code::Ok, bsoncxx::to_json(*result));
        } else {
            resp.send(Http::Code::Not_Found, "No encontrado");
        }
    }

    // POST /data
    // POST /data
void insertData(const Rest::Request& req, Http::ResponseWriter resp) {
    setCorsHeaders(resp);
    
    std::cout << "📥 Received POST /data with body: " << req.body() << std::endl;
    
    try {
        auto body = json::parse(req.body());
        std::vector<std::string> data;

        std::cout << "🔍 Parsed JSON has " << body.size() << " elements" << std::endl;
        
        // Debug: imprimir todas las claves y valores
        for (auto it = body.begin(); it != body.end(); ++it) {
            std::cout << "Key: " << it.key() << ", Value type: " << it.value().type_name() << std::endl;
            data.push_back(it.key());
            
            // Convertir cualquier tipo a string de forma segura
            if (it.value().is_string()) {
                data.push_back(it.value().get<std::string>());
            } else if (it.value().is_number()) {
                data.push_back(std::to_string(it.value().get<int>()));
            } else if (it.value().is_boolean()) {
                data.push_back(it.value().get<bool>() ? "true" : "false");
            } else {
                data.push_back(it.value().dump()); // convertir cualquier tipo a string JSON
            }
        }

        std::cout << "📤 Inserting document with " << data.size() << " fields" << std::endl;
        
        if (handler->InsertDocument(data.data(), data.size())) {
            std::cout << "✅ Document inserted successfully" << std::endl;
            resp.send(Http::Code::Created, "Insertado");
        } else {
            std::cout << "❌ InsertDocument returned false" << std::endl;
            resp.send(Http::Code::Bad_Request, "Error en inserción");
        }
    } catch (const std::exception& e) {
        std::cout << "💥 Exception in insertData: " << e.what() << std::endl;
        resp.send(Http::Code::Bad_Request, "JSON inválido: " + std::string(e.what()));
    }
}

    // PUT /data/:field/:value
    void updateData(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        try {
            auto body = json::parse(req.body());
            if (body.size() != 1) {
                resp.send(Http::Code::Bad_Request, "Debe enviar solo un campo para actualizar");
                return;
            }

            auto upd_field = body.begin().key();
            auto upd_value = body.begin().value().get<std::string>();

            handler->UpdateData(field, value, upd_field, upd_value);
            resp.send(Http::Code::Ok, "Actualizado");
        } catch (const std::exception& e) {
            resp.send(Http::Code::Bad_Request, "JSON inválido: " + std::string(e.what()));
        }
    }

    // DELETE /data/:field/:value
    void deleteData(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        handler->DeleteData(field, value);
        resp.send(Http::Code::Ok, "Eliminado");
    }

    void setCorsHeaders(Pistache::Http::ResponseWriter& response) {
        response.headers()
            .add<Pistache::Http::Header::AccessControlAllowOrigin>("*")
            .add<Pistache::Http::Header::AccessControlAllowMethods>("GET, POST, PUT, DELETE, OPTIONS")
            .add<Pistache::Http::Header::AccessControlAllowHeaders>("Content-Type, Authorization");
        // AccessControlMaxAge no es soportado por Pistache, lo removemos
    }

    // Maneja preflight OPTIONS
    void handleOptions(const Rest::Request& req, Http::ResponseWriter resp) {
        setCorsHeaders(resp);
        resp.send(Http::Code::Ok);
    }

public:
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    std::shared_ptr<hdb::MongoHandler> handler;
};

int main() {
    mongocxx::instance instance{};
    Port port(9080);
    Address addr(Ipv4::any(), port);
    
    auto dbHandler = std::make_shared<hdb::MongoHandler>("test_db", "test_coll");

    RestApi api(addr, dbHandler);
    api.init(2);
    std::cout << "Servidor corriendo en http://localhost:9080\n";
    std::cout << "Documentos en colección: " << dbHandler->coll.count_documents({}) << "\n";
    api.start();

    return 0;
}