#include <iostream>
#include <fstream>
#include <sstream>

#include "Parser.h"
#include "visitors/CodegenVisitor.h"
#include "Extension.h"

int main(int argc, char** argv) {
    // Simple blinking program
    // std::string program = "require neopixel;\nloop {\n\tneopixel.fill_rgb(255, 0, 0);\n\tneopixel.show();\n\tdelay(500);\n\tneopixel.fill_rgb(0, 255, 0);\n\tneopixel.show();\n\tdelay(500);\n}";
    
    if (argc < 3) {
        std::cerr << "Usage: LumaC <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::ifstream inFile(argv[1]);
    std::stringstream buf;
    buf << inFile.rdbuf();
    
    Parser parser(buf.str());
    Program* prog = parser.parse();

    ExtensionRegistry::instance().registerExt("neopixel", std::make_unique<Neopixel>());
    ExtensionRegistry::instance().registerExt("microphone", std::make_unique<Microphone>());

    CodegenVisitor cgv;
    cgv.visitProgram(prog);
    auto code = cgv.getLBC();

    FILE* outFile = fopen(argv[2], "wb");
    fwrite(code.data(), sizeof(code[0]), code.size(), outFile);

    std::cout << prog->to_string() << std::endl;
    for (size_t i = 0; i < code.size(); i++) {
        printf("%02X ", code[i]);
    }

    return 0;
}