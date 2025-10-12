#pragma once
#include<iostream>
#include<unordered_map>
namespace User
{
   
    inline const std::string name =         "name";
    inline const std::string lastName =         "last_name";
    inline const std::string birthDate =        "birthdate";
    inline const std::string age =        "age";
    inline const std::string cellphone =        "cellphone";
    inline const std::string email =        "email";
    inline const std::string password =         "password";
    inline const std::string nroDoc =         "numDoc";
    inline const std::string tipoDoc =        "typeDoc";
    inline const std::string rol = "rol";

    
}

using string = std::string;
struct user
{
  public:
      std::string name;
      std::string lastName;
      std::string birthDate;
      std::string age;
      std::string cellphone;
      std::string email;
      std::string password;
      std::string nroDoc;
      std::string tipoDoc;
      std::string rol;
      inline user(string email1,
      string password1,string name1 = "N/A", string lastName1 = "N/A", string birthDate1 = "N/A", string age1 = "N/A", string cellphone1 = "N/A",
       string nroDoc1 = "N/A", string tipoDoc1 = "N/A", string rol1 = "N/A") : name(name1), lastName(lastName1),
      birthDate(birthDate1), age(age1), cellphone(cellphone1), email(email1), password(password1), 
      nroDoc(nroDoc1), tipoDoc(tipoDoc1), rol(rol1) 
      {}

};