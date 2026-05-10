#pragma once
#include<string>
#include<mongocxx/pool.hpp>
#include<mongocxx/collection.hpp>

struct poolhandle{
    mongocxx::pool::entry session;
    mongocxx::collection coll;
    poolhandle(mongocxx::pool::entry&& e,mongocxx::collection col):
    session(std::move(e)),coll(col){}
};