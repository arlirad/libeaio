add_library(eaio STATIC dispatcher.cpp handle.cpp)

target_sources(eaio PRIVATE
    handle/file.cpp
    handle/signal.cpp
    handle/socket.cpp
)

include_directories(./)

target_compile_options(eaio PRIVATE -fcoroutines)