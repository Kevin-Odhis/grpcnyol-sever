cmake_minimum_required(VERSION 3.30)
project(nyolapp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

#set(CMAKE_CXX_FLAGS_RELEASE
   # "${CMAKE_CXX_FLAGS_RELEASE} -g"
#)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(CMAKE_FIND_PACKAGE_WARN_NO_MODULE TRUE)

find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_executable(nyolapp mongo.cpp mongo.h poolhandle.h
        server.cpp server.h
        ActivityLogger.cpp ActivityLogger.h
        ActivityInterceptor.cpp ActivityInterceptor.h
        LogRecord.h
       
        Tutor.h Tutor.cpp 
        School.grpc.pb.h School.pb.h School.grpc.pb.cc School.pb.cc
        Student.h Student.cpp
        )

target_link_libraries(nyolapp PRIVATE
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    gRPC::grpc++
    protobuf::libprotobuf
    #gRPC::grpc++_reflection
)

if(win32)
    target_sources(nyolapp PRIVATE app.rc)
endif()