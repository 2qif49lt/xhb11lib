#ifndef XHBLIB_REFLECT_H_
#define XHBLIB_REFLECT_H_

#include <string>
#include <sstream>
#include <type_traits>
#include <vector>
#include <cstddef> // offsetof
#include <cstdint> // uint32_t ...

namespace xhb {

struct type_descriptor {
    const char* _name;
    size_t _size;

    type_descriptor(const char* name,size_t size) : _name{name}, _size{size} {}
    virtual ~type_descriptor() {}
    virtual std::string name() const { return _name; }
    size_t size() const { return _size; }
    virtual std::string dump(const void* obj, int lev = 0) const = 0; 
};

// 获取非符合自定义规范的类型，比如int,vector.
template <typename T>
type_descriptor* get_primitive_descri() {
    struct type_descripotr_unsupport : type_descriptor {
        char _typename[256];
        type_descripotr_unsupport(const char* name, size_t size) : 
            type_descriptor(nullptr, sizeof(T)) {
                strcpy(_typename, typeid(T).name());
                _name = _typename;
            }
        virtual std::string dump(const void* obj, int lev) const override {
            std::stringstream ss;
            ss << name() << "{unsupported,size:" << size() << "}";
            return ss.str();
        } 

    };
    static type_descripotr_unsupport desc{nullptr, sizeof(T)};
    return &desc;
}

namespace reflect_impl {

// 判断是否是自定义的反射类。
template <typename T>
class is_builtin_reflect {
private:
    using one = char;
    using two = long;

    template <typename U> static one sfinae(decltype(&U::_xhb_reflect));
    template <typename U> static two sfinae(...);

public:
    enum { value = sizeof(sfinae<T>(nullptr)) == sizeof(one),};
};

struct type_helper {
    template <typename T, typename std::enable_if<is_builtin_reflect<T>::value>::type* = nullptr>
    static type_descriptor* get() {
        return &T::_xhb_reflect;
    }

    template <typename T, typename std::enable_if<!is_builtin_reflect<T>::value>::type* = nullptr>
    static type_descriptor* get() {
        return get_primitive_descri<T>();
    }
};


} // reflect_impl ns

template <typename T>
struct type_resolver {
    static type_descriptor* get() {
        return reflect_impl::type_helper::get<T>();
    }
};

struct type_descri_struct : type_descriptor {
    struct member {
        const char* _name;
        size_t _offset;
        type_descriptor* descri;
    };

    std::vector<member> _members;
    
    type_descri_struct(void (*init)(type_descri_struct*)) : type_descriptor(nullptr, 0) {
        init(this);
    }

    virtual std::string dump(const void* obj, int lev = 0) const override {
        std::stringstream ss;
        ss << _name << " {\n";
        for (const member& m : _members) {
            ss << std::string(4 * (lev + 1), ' ') << m._name << " = " << 
                m.descri->dump((char*)obj + m._offset, lev + 1) << '\n';
        }
        ss << std::string(4 * lev, ' ') << "}";
        return ss.str();
    }
};

#define XHB_REFLECT() \
    friend struct xhb::reflect_impl::type_helper; \
    static xhb::type_descri_struct _xhb_reflect; \
    static void xhb_init_reflect(xhb::type_descri_struct*);

#define XHB_REFLECT_STRUCT_BEGIN(type) \
    xhb::type_descri_struct type::_xhb_reflect{&type::xhb_init_reflect}; \
    void type::xhb_init_reflect(xhb::type_descri_struct* s) { \
        using T = type; \
        s->_name = #type; \
        s->_size = sizeof(type); \
        s->_members = {

#define XHB_REFLECT_STRUCT_MEMBER(name) \
            {#name, offsetof(T, name), xhb::type_resolver<decltype(T::name)>::get()},

#define XHB_REFLECT_STRUCT_END() \
        }; \
    }


} // xhb ns
#endif // XHBLIB_REFLECT_H_