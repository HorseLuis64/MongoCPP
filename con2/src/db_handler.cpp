#include <db_handler.h>


namespace hdb
{
    MongoHandler::MongoHandler(const std::string dbName, const std::string default_collection)
    {
      uri = mongocxx::uri(default_uri);
      client = mongocxx::client(uri);
      db = client[dbName];
      coll = db[default_collection];
    }

    bool MongoHandler::InsertDocument(std::string field, std::string data)
    {
          bsoncxx::builder::stream::document doc{};
          doc << field << data;
          if(coll.insert_one(doc.view()))
          {
                std::cout<<"yupuu\n";
                return true;
          }
          std::cout<<"erro papi\n";
          return false;
    }

    bool MongoHandler::InsertDocument(std::string field_data[], int size)
    {
      
      if(size % 2 != 0)
      {
        std::cout<<"par papi, par";
          return false;
      }

      bsoncxx::builder::stream::document doc;

      for(int i = 0; i < size; i+=2)
      {
          doc << field_data[i] << field_data[i + 1];
      }
      std::cout<<"todo colecto\n";
      coll.insert_one(doc.view());
      return true;
      
    }

    bool MongoHandler::GetAllData(mongocxx::cursor &cursor)
    {
      cursor = coll.find({});
      return true;
    }

    bsoncxx::stdx::optional<bsoncxx::document::value> MongoHandler::GetOneData(std::string field, std::string data)
    {
      bsoncxx::builder::stream::document doc;
      doc << field << data;
      auto result = coll.find_one(doc.view());
      if(result)
      {
        std::cout<<bsoncxx::to_json(*result)<<"\n";
      }
      else
      {
          std::cout<<"lo siento pa, no se encontro"<<std::endl;
      }
      return result;
    }
    void MongoHandler::ShowDataOnCursor(mongocxx::cursor &cursor)
    {
      for(auto&& doc : cursor)
      {
        std::cout<<bsoncxx::to_json(doc)<<"\n";
      }
    }

    bool MongoHandler::UpdateData(std::string filter_field, std::string filter_data, std::string upd_field, std::string upd_data)
    {
      bsoncxx::builder::stream::document filter, update;
      filter <<filter_field<<filter_data;
      update << "$set" << bsoncxx::builder::stream::open_document <<
      upd_field << upd_data <<
      bsoncxx::builder::stream::close_document;

      coll.update_one(filter.view(), update.view());

      return true;
    }

    void MongoHandler::DeleteData(std::string filter_field, std::string filter_data)
    {
      bsoncxx::builder::stream::document filter;
      filter << filter_field << filter_data;
      coll.delete_one(filter.view());
    }

    bool MongoHandler::UpdateAllData(std::string filter_field, std::string filter_data, std::string upd_field, std::string upd_data)
    {
      bsoncxx::builder::stream::document filter, update;
      filter <<filter_field<<filter_data;
      update << "$set" << bsoncxx::builder::stream::open_document <<
      upd_field << upd_data <<
      bsoncxx::builder::stream::close_document;

      coll.update_many(filter.view(), update.view());

      return true;
    }

    void MongoHandler::DeleteAllData(std::string filter_field, std::string filter_data)
    {
      bsoncxx::builder::stream::document filter;
      filter << filter_field << filter_data;
      coll.delete_many(filter.view());
    }

}
