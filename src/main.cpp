#include "db_handler.h"
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <bsoncxx/json.hpp>
using namespace Pistache;

class HelloHandler : public Http::Handler {
public:
    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request& request, Http::ResponseWriter response) override {
        response.send(Http::Code::Ok, "Hola Mundo!\n");
    }
};

int main() 
{
  std::string fields[] = 
  {"size", "3",
    "name", "pedro",
    "money", "no"
  };
  mongocxx::instance instance{};
  hdb::MongoHandler han("CARAJO_CURLY", "eynometo");
  han.InsertDocument(fields, 6);
  mongocxx::cursor cursor = han.coll.find({});
  han.GetOneData("oh si curly", "no, por el pilin no");
  han.UpdateAllData("name", "pedro", "name", "pipensio777");
  han.ShowDataOnCursor(cursor);
    
}
