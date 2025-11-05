#ifndef LUMA_EXTENSION_H
#define LUMA_EXTENSION_H

#include <stdint.h>
#include <string>
#include <memory>
#include <unordered_map>

struct ExtFunction {
    uint8_t subOp;
    size_t argCount;
    bool hasReturnValue;
};

class Extension {
    protected:
        std::unordered_map<std::string, ExtFunction> functions;
        uint8_t id;

    public:
        uint8_t getID() {
            return id;
        }

        ExtFunction* getFunction(std::string name) {
            auto it = functions.find(name);
            if (it == functions.end()) return nullptr;
            return &it->second;
        }
};

class Neopixel : public Extension {
    public:
        Neopixel() {
            id = 0x01;
            
            functions.insert({"set_rgb", {0x00, 4, false}});
            functions.insert({"fill_rgb", {0x01, 3, false}});
            functions.insert({"show", {0x02, 0, false}});
            functions.insert({"clear", {0x03, 0, false}});
            functions.insert({"num_leds", {0x04, 0, true}});
        }
};

class Microphone : public Extension {
    public:
        Microphone() {
            id = 0x02;
            functions.insert({"read", {0x00, 0, true}});
        }
};

class ExtensionRegistry {
    std::unordered_map<std::string, std::unique_ptr<Extension>> exts;

    public:
        static ExtensionRegistry& instance() {
            static ExtensionRegistry inst;
            return inst;
        }

        void registerExt(const std::string& name, std::unique_ptr<Extension> ext) {
            exts[name] = std::move(ext);
        }

        Extension* get(const std::string& name) {
            auto it = exts.find(name);
            return it != exts.end() ? it->second.get() : nullptr;
        }
};

#endif