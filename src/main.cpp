#include "crow_all.h"
#include "algorithms.h"
#include <iostream>

int main(){
    crow::SimpleApp app;
    DataCleaner cleaner;

    CROW_ROUTE(app,"/")
    ([](){
        return "Data Cleaning Toolkit API is running :)";
    });
}