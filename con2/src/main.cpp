#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <iostream>
#include "json.hpp"   
#include <fstream> //Para manejar JSON (https://github.com/nlohmann/json)
#include "db_handler.h"

using json = nlohmann::json;
using namespace Pistache;

class RestApi {
public:
    RestApi(Address addr, std::shared_ptr<hdb::MongoHandler> dbHandler)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)), handler(dbHandler) {}

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(thr));
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

    // Rutas de la API
    Routes::Get(router, "/data", Routes::bind(&RestApi::getAllData, this));
    Routes::Get(router, "/data/:field/:value", Routes::bind(&RestApi::getOneData, this));
    Routes::Post(router, "/data", Routes::bind(&RestApi::insertData, this));
    Routes::Put(router, "/data/:field/:value", Routes::bind(&RestApi::updateData, this));
    Routes::Delete(router, "/data/:field/:value", Routes::bind(&RestApi::deleteData, this));

    // Ruta estática para frontend
    Routes::Get(router, "/*", Routes::bind(&RestApi::serveStatic, this));
    

}

void serveStatic(const Rest::Request& req, Http::ResponseWriter resp) {
    auto path = req.resource();  
    if (path == "/" || path.empty()) {
        path = "/index.html";
    }

    std::string fullPath = "static" + path;

    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        resp.send(Http::Code::Not_Found, "Archivo no encontrado");
        return;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    resp.send(Http::Code::Ok, ss.str());
}


    // GET /data
    void getAllData(const Rest::Request& req, Http::ResponseWriter resp) {
        mongocxx::cursor cursor = handler->defaultCursor();
        handler->GetAllData(cursor);

        json arr = json::array();
        for (auto&& doc : cursor) {
            arr.push_back(json::parse(bsoncxx::to_json(doc)));
        }
        setCorsHeaders(resp);
        resp.send(Http::Code::Ok, arr.dump());
    }

    // GET /data/:field/:value
    void getOneData(const Rest::Request& req, Http::ResponseWriter resp) {
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        auto result = handler->GetOneData(field, value);
        if (result) {
            setCorsHeaders(resp);
            resp.send(Http::Code::Ok, bsoncxx::to_json(*result));
        } else {
            setCorsHeaders(resp);
            resp.send(Http::Code::Not_Found, "No encontrado");
        }
    }

    // POST /data
    void insertData(const Rest::Request& req, Http::ResponseWriter resp) {
        try {
            auto body = json::parse(req.body());
            std::vector<std::string> data;

            for (auto it = body.begin(); it != body.end(); ++it) {
                data.push_back(it.key());
                data.push_back(it.value().get<std::string>());
            }

            if (handler->InsertDocument(data.data(), data.size())) {
                setCorsHeaders(resp);
                resp.send(Http::Code::Created, "Insertado");
            } else {
                setCorsHeaders(resp);
                resp.send(Http::Code::Bad_Request, "Error en inserción");
            }
        } catch (...) {
            setCorsHeaders(resp);
            resp.send(Http::Code::Bad_Request, "JSON inválido");
        }
    }

    // PUT /data/:field/:value
    void updateData(const Rest::Request& req, Http::ResponseWriter resp) {
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        try {
            auto body = json::parse(req.body());
            if (body.size() != 1) {
                setCorsHeaders(resp);
                resp.send(Http::Code::Bad_Request, "Debe enviar solo un campo para actualizar");
                return;
            }

            auto upd_field = body.begin().key();
            auto upd_value = body.begin().value().get<std::string>();

            handler->UpdateData(field, value, upd_field, upd_value);

            setCorsHeaders(resp);
            resp.send(Http::Code::Ok, "Actualizado");
        } catch (...) {
            setCorsHeaders(resp);
            resp.send(Http::Code::Bad_Request, "JSON inválido");
        }
    }

    // DELETE /data/:field/:value
    void deleteData(const Rest::Request& req, Http::ResponseWriter resp) {
        auto field = req.param(":field").as<std::string>();
        auto value = req.param(":value").as<std::string>();

        handler->DeleteData(field, value);
        setCorsHeaders(resp);
        resp.send(Http::Code::Ok, "Eliminado");
    }
    void setCorsHeaders(Pistache::Http::ResponseWriter& response) {
    response.headers()
        .add<Pistache::Http::Header::AccessControlAllowOrigin>("*")
        .add<Pistache::Http::Header::AccessControlAllowMethods>("GET, POST, PUT, DELETE, OPTIONS")
        .add<Pistache::Http::Header::AccessControlAllowHeaders>("Content-Type");
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
    std::cout << "Servidor corriendo en http://localhost:9080 docs: " << api.handler->coll.count_documents({}) << "\n";
    std::cout<<"docs: "<< api.handler->coll.count_documents({});
    api.start();
    

    return 0;
}
