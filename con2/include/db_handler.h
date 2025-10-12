#pragma once
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include<bsoncxx/json.hpp>
#include <unordered_map>
#include "../../datafields.h"

namespace hdb {
  inline const std::string default_uri = "mongodb://localhost:27017";
  
  class MongoHandler {
  public:
      mongocxx::client client;
      mongocxx::database db;
      mongocxx::collection coll;
      mongocxx::collection users;
      mongocxx::collection products;
      mongocxx::uri uri;
      std::string dbName;
      std::string defaultCollection;


  public:
      MongoHandler(const std::string dbName, const std::string defaultCollection);
  
      bool InsertDocument(const std::string field, const std::string data);
      bool InsertDocument(std::string field_data[], int size);
      bool GetAllData(mongocxx::cursor &cursor);
      bsoncxx::stdx::optional<bsoncxx::document::value> GetOneData(std::string field, std::string data);
      void ShowDataOnCursor(mongocxx::cursor &cursor);
      bool UpdateData(std::string filter_field, std::string filter_data, std::string upd_field, std::string upd_data);
      void DeleteData(std::string filter_field, std::string filter_data);
      bool UpdateAllData(std::string filter_field, std::string filter_data, std::string upd_field, std::string upd_data);
      void DeleteAllData(std::string filter_field, std::string filter_data);
      mongocxx::cursor defaultCursor()
      {
        bsoncxx::builder::stream::document doc;
        doc << "omla" << "2";
        return coll.find(doc.view());
      }



      //TODO:: REAL DOCUMENTS METHODS
      bool InsertRegister(const std::unordered_map<User::fields, std::string> &reg);
      bool InsertRegister(const user &reg);
      bool VerifyLogin(const std::string email, const std::string password);

  };  

  
} // namespace hdb



