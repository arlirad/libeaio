add_library(eaio STATIC dispatcher.cpp handle.cpp)
target_compile_options(eaio PRIVATE -fcoroutines)