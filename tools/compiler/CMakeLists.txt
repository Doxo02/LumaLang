add_executable(LumaC main.cpp Tokenizer.cpp Parser.cpp visitors/CodegenVisitor.cpp)
target_include_directories(LumaC PUBLIC "../../common")