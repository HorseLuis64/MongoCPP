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


    using field = User::fields;
    bool MongoHandler::InsertRegister(const std::unordered_map<User::fields, std::string> &reg)
    {
      bsoncxx::builder::stream::document doc;
      doc 
      << User::name       << reg.at(field::_name)
      << User::lastName   << reg.at(field::_lastName)
      << User::age        << reg.at(field::_age)
      << User::birthDate  << reg.at(field::_birthDate)
      << User::cellphone  << reg.at(field::_cellphone)
      << User::email      << reg.at(field::_email)
      << User::password   << reg.at(field::_password);

      users.insert_one(doc.view());
      std::cout<<"yey insertado registro";
      return true;
    }

    bool MongoHandler::InsertRegister(const user &reg)
    {
      bsoncxx::builder::stream::document doc;
      doc 
      << User::name       << reg.name
      << User::lastName   << reg.lastName
      << User::age        << reg.age
      << User::birthDate  << reg.birthDate
      << User::cellphone  << reg.cellphone
      << User::email      << reg.email
      << User::password   << reg.password;

      users.insert_one(doc.view());
      std::cout<<"ingresado por objeto yupi\n";
      return true;
    }

}
