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
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    std::shared_ptr<hdb::MongoHandler> handler;
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

                

        //TODO: RUTAS REALES
        Routes::Options(router, "/*", Routes::bind(&RestApi::handleOptions, this));
        Routes::Post(router, "/login", Routes::bind(&RestApi::postLogin, this));
        Routes::Post(router, "/register", Routes::bind(&RestApi::postRegister, this));
        Routes::Options(router, "/register", Routes::bind(&RestApi::handleOptions, this));
        Routes::Post(router, "/products", Routes::bind(&RestApi::postProduct, this));

        
        
    }

    

    // GET /data
    
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

    //TODO: MAKE POST/GET/UPDATE ROUTES
    void postRegister(const Rest::Request& req, Http::ResponseWriter resp)
    {
      setCorsHeaders(resp);
      std::string file = req.body();
      json data = json::parse(file);

       user fields
       (
            data[User::name],
            data[User::lastName],
            data[User::birthDate],
            data[User::age],
            data[User::cellphone],
            data[User::email],
            data[User::password],
            data[User::nroDoc],
            data[User::tipoDoc],
            data[User::rol]
       );
        
        handler->InsertRegister(fields);

        resp.send(Pistache::Http::Code::Ok, "Registro recibido");

    }
    void postLogin(const Rest::Request& req, Http::ResponseWriter resp)
    {

    }
    void postProduct(const Rest::Request& req, Http::ResponseWriter resp)
    {

    }

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