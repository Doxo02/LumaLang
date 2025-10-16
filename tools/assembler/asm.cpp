#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <iomanip>

#include "../../common/opcode.h"

struct Label {
    std::string name;
    uint16_t addr;
};

// Simple utility for writing bytes to a vector
class ByteWriter {
public:
    std::vector<uint8_t> data;
    std::vector<uint8_t> extensions;
    std::vector<Label> labels;

    void require(uint8_t id) {
        extensions.push_back(id);
    }

    void emit(uint8_t byte) { data.push_back(byte); }

    void emit16(uint16_t val) {
        for (int i = 0; i < 2; i++)
            data.push_back((val >> (i * 8)) & 0xFF);
    }

    void emit32(uint32_t val) {
        for (int i = 0; i < 4; i++)
            data.push_back((val >> (i * 8)) & 0xFF);
    }

    void newLabel(const std::string& name) {
        labels.push_back(Label{name, (uint16_t) data.size()});
    }

    void jump(const std::string& name, uint8_t op) {
        Label* label = nullptr;
        for (int i = 0; i < labels.size(); i++) {
            if (labels[i].name == name) {
                label = &labels[i];
                break;
            }
        }
        if (label == nullptr) {
            throw std::runtime_error("Unknown label: " + name);
        }
        emit(op);
        emit16(label->addr);
    }

    size_t writeToFile(const std::string& filename) {
        uint8_t head[16];
        for (int i = 0; i < 16; i++) {
            head[i] = 0;
        }
        std::vector<uint8_t> extTable;
        std::cout << extensions.size() << ": " << extensions.size() * 3 << std::endl;
        // Header
        head[0] = 'L'; head[1] = 'V'; head[2] = 'M'; head[3] = '1';     // Magic Number
        head[4] = 0x01;                                                 // Version
        head[6] = (uint8_t) extensions.size();                          // ExtCount

        uint16_t codeOffset = (uint16_t) (16 + extensions.size() * 3);
        head[8] = (uint8_t) (codeOffset & 0xFF);
        head[9] = (uint8_t) ((codeOffset >> 8) & 0xFF);                 // code offset

        // Extension table
        for (uint8_t id : extensions) {
            extTable.push_back(id);
            extTable.push_back(0);
            extTable.push_back(0);
        }

        std::ofstream out(filename, std::ios::binary);
        out.write(reinterpret_cast<const char*>(head), 16);

        if (extTable.size() > 0) {
            out.write(reinterpret_cast<const char*>(extTable.data()), extTable.size());
        }

        out.write(reinterpret_cast<const char*>(data.data()), data.size());

        return 16 + data.size() + extTable.size();
    }
};

// Trim whitespace helper
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Register parsing
int parseRegister(const std::string& token) {
    if (token.size() < 2 || toupper(token[0]) != 'R')
        throw std::runtime_error("Invalid register: " + token);
    int r = std::stoi(token.substr(1));
    if (r < 0 || r > 7)
        throw std::runtime_error("Register out of range (R0-R7): " + token);
    return r;
}

ByteWriter w;

// Main assembly function
std::vector<uint8_t> assemble(std::istream& in) {
    std::string line;
    int lineNum = 0;

    while (std::getline(in, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == ';') continue; // comment or blank

        std::istringstream iss(line);
        std::string op;
        iss >> op;

        for (auto& c : op) c = toupper(c);

        if (op == "REQ") {
            std::string idStr;
            iss >> idStr;
            uint8_t id = (uint8_t) std::stoul(idStr);
            w.require(id);
        }
        else if (op == "MOVI") {
            std::string rd, immStr;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> immStr;

            int reg = parseRegister(rd);
            uint32_t imm = std::stoul(immStr);

            w.emit(OP_MOVI);
            w.emit(reg);
            w.emit32(imm);
        }
        else if (op == "MOV") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;
            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_MOV);
            w.emit(dstsrc);
        }
        else if (op == "LOAD") {
            std::string rd, addrStr;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> addrStr;

            int reg = parseRegister(rd);
            uint8_t addr = (uint8_t) std::stoul(addrStr);

            w.emit(OP_LOAD);
            w.emit(reg);
            w.emit(addr);
        }
        else if (op == "STORE") {
            std::string rd, addrStr;
            iss >> addrStr;
            if (iss.peek() == ',') iss.ignore();
            iss >> rd;

            int reg = parseRegister(rd);
            uint8_t addr = (uint8_t) std::stoul(addrStr);

            w.emit(OP_STORE);
            w.emit(addr);
            w.emit(reg);
        }
        else if (op == "PUSH") {
            std::string rd;
            iss >> rd;
            int reg = parseRegister(rd);
            w.emit(OP_PUSH);
            w.emit(reg);
        }
        else if (op == "POP") {
            std::string rd;
            iss >> rd;
            int reg = parseRegister(rd);
            w.emit(OP_POP);
            w.emit(reg);
        }
        else if (op == "LDC") {
            std::string rd, idxStr;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> idxStr;

            int reg = parseRegister(rd);
            uint8_t idx = (uint8_t) std::stoul(idxStr);

            w.emit(OP_LDC);
            w.emit(reg);
            w.emit(idx);
        }
        else if (op == "ADD") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_ADD);
            w.emit(dstsrc);
        }
        else if (op == "SUB") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_SUB);
            w.emit(dstsrc);
        }
        else if (op == "MUL") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_MUL);
            w.emit(dstsrc);
        }
        else if (op == "DIV") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_DIV);
            w.emit(dstsrc);
        }
        else if (op == "MOD") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_MOD);
            w.emit(dstsrc);
        }
        else if (op == "ABS") {
            std::string rd;
            iss >> rd;

            int reg = parseRegister(rd);

            w.emit(OP_ABS);
            w.emit(reg);
        }
        else if (op == "MAX") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_MAX);
            w.emit(dstsrc);
        }
        else if (op == "MIN") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_MIN);
            w.emit(dstsrc);
        }
        else if (op == "AND") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_AND);
            w.emit(dstsrc);
        }
        else if (op == "OR") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_OR);
            w.emit(dstsrc);
        }
        else if (op == "XOR") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_XOR);
            w.emit(dstsrc);
        }
        else if (op == "NOT") {
            std::string rd;
            iss >> rd;

            int reg = parseRegister(rd);

            w.emit(OP_NOT);
            w.emit(reg);
        }
        else if (op == "EQ") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_EQ);
            w.emit(dstsrc);
        }
        else if (op == "NEQ") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_NEQ);
            w.emit(dstsrc);
        }
        else if (op == "GEQ") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_GEQ);
            w.emit(dstsrc);
        }
        else if (op == "LEQ") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_LEQ);
            w.emit(dstsrc);
        }
        else if (op == "GT") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_GT);
            w.emit(dstsrc);
        }
        else if (op == "LT") {
            std::string rd, rs;
            iss >> rd;
            if (iss.peek() == ',') iss.ignore();
            iss >> rs;

            int dst = parseRegister(rd);
            int src = parseRegister(rs);
            uint8_t dstsrc = (dst << 4) | (src & 0xF);

            w.emit(OP_LT);
            w.emit(dstsrc);
        }
        else if (op == "JMP") {
            std::string name;
            iss >> name;
            for (auto& c : name) c = toupper(c);
            w.jump(name, OP_JMPA);
        }
        else if (op == "JZ") {
            std::string name;
            iss >> name;
            for (auto& c : name) c = toupper(c);
            w.jump(name, OP_JZA);
        }
        else if (op == "JNZ") {
            std::string name;
            iss >> name;
            for (auto& c : name) c = toupper(c);
            w.jump(name, OP_JNZA);
        }
        else if (op == "CALL") {
            std::string name;
            iss >> name;
            for (auto& c : name) c = toupper(c);
            w.jump(name, OP_CALLA);
        }
        else if (op == "RET") {
            w.emit(OP_RET);
        }
        else if (op == "FRGB") {
            w.emit(OP_D_FRGB);
        }
        else if (op == "SHOW") {
            w.emit(OP_D_SHOW);
        }
        else if (op == "DELAY") {
            std::string rd;
            iss >> rd;
            int reg = parseRegister(rd);
            w.emit(OP_DELAY);
            w.emit(reg);
        }
        else if (op == "HALT") {
            w.emit(OP_HALT);
        }
        else if (op.back() == ':') {
            op.pop_back();
            w.newLabel(op);
        }
        else {
            throw std::runtime_error("Unknown opcode '" + op + "' on line " + std::to_string(lineNum));
        }
    }

    return w.data;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: assembler <input.asm> <output.lbc>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Failed to open input file: " << argv[1] << "\n";
        return 1;
    }

    try {
        auto code = assemble(in);

        size_t codeSize = w.writeToFile(argv[2]);
        std::cout << "Assembled " << codeSize << " bytes -> " << argv[2] << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Assembly error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
