#ifndef XHBLIB_REFLECT_PRIMITIVES_H_
#define XHBLIB_REFLECT_PRIMITIVES_H_

#include "reflect.h"

namespace xhb {

#define XHB_REFLECT_INIT_PRIMITIVE(type) \
    struct type_descriptor_##type : type_descriptor { \
        type_descriptor_##type() : type_descriptor(#type, sizeof( type )) {} \
        virtual std::string dump(const void* obj, int lev) const override { \
            std::stringstream ss; \
            auto p = static_cast<const type *>(obj); \
            ss << name() << "{" << *p << "}"; \
            return ss.str(); \
        } \
    }; \
    template<> \
    type_descriptor* get_primitive_descri<type>() { \
        static type_descriptor_##type desc; \
        return &desc; \
    }


XHB_REFLECT_INIT_PRIMITIVE(int8_t)
XHB_REFLECT_INIT_PRIMITIVE(uint8_t)

XHB_REFLECT_INIT_PRIMITIVE(int16_t)
XHB_REFLECT_INIT_PRIMITIVE(uint16_t)

XHB_REFLECT_INIT_PRIMITIVE(int)
XHB_REFLECT_INIT_PRIMITIVE(uint32_t)

XHB_REFLECT_INIT_PRIMITIVE(long)

XHB_REFLECT_INIT_PRIMITIVE(int64_t)
XHB_REFLECT_INIT_PRIMITIVE(uint64_t)

XHB_REFLECT_INIT_PRIMITIVE(float)
XHB_REFLECT_INIT_PRIMITIVE(double)



struct type_descriptor_char : type_descriptor {
    type_descriptor_char() : type_descriptor("char", sizeof(char)) {}
    virtual std::string dump(const void* obj, int lev) const override {
        std::stringstream ss; 
        auto p = static_cast<const char *>(obj); 
        ss << name() << "{'" << *p << "'}"; 
        return ss.str();
    }
};

template<>
type_descriptor* get_primitive_descri<char>() {
    static type_descriptor_char desc;
    return &desc;
}

struct type_descriptor_voidptr : type_descriptor {
    type_descriptor_voidptr() : type_descriptor("void*", sizeof(void*)) {}
    virtual std::string dump(const void* obj, int lev) const override {
        return "void*{...}";
    }
};

template<>
type_descriptor* get_primitive_descri<void*>() {
    static type_descriptor_voidptr desc;
    return &desc;
}


} // xhb ns
#endif // XHBLIB_REFLECT_PRIMITIVES_H_