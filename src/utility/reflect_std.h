#ifndef XHBLIB_REFLECT_VECTOR_H_
#define XHBLIB_REFLECT_VECTOR_H_

#include "reflect.h"


namespace xhb {

// vector
struct type_descriptor_vec : type_descriptor {
    type_descriptor* _desc;
    size_t (*_length)(const void*);
    const void* (*_item)(const void*, size_t);

    template <typename I>
    type_descriptor_vec(I*) : type_descriptor{"std::vector<>", sizeof(std::vector<I>)}, 
        _desc{type_resolver<I>::get()} {

        _length = [](const void* ptr) -> size_t {
            auto vec = static_cast<const std::vector<I>*>(ptr);
            return vec->size();
        };

        _item = [](const void* ptr, size_t idx) -> const void* {
            auto vec = static_cast<const std::vector<I>*>(ptr);
            return &((*vec)[idx]);
        };
    }
    virtual std::string name() const override {
        return std::string("std::vector<") + _desc->name() + ">";
    }
    
    virtual std::string dump(const void* obj, int lev) const override {
        auto len = _length(obj);
        std::stringstream ss;

        ss << name();

        if (len == 0) {
            ss << "{}";
        } else {
            ss << "{\n";
            for (size_t idx = 0; idx != len; ++idx) {
                ss << std::string(4 * (lev + 1), ' ') << '[' << idx << ']';
                ss << _desc->dump(_item(obj, idx), lev + 1);
                ss << '\n';
            }
            ss << std::string(4 * lev, ' ') << "}";
        }
        return ss.str();
    }
};

template <typename T>
struct type_resolver<std::vector<T>> {
    static type_descriptor* get() {
        static type_descriptor_vec _desc{(T*)nullptr};
        return &_desc;
    }
};

// string
struct type_descriptor_string : type_descriptor {
    type_descriptor_string() :type_descriptor("std::string", sizeof(std::string)) {}
    virtual std::string dump(const void* obj, int lev) const override {
        std::stringstream ss;
        ss << "std::string{\"" << *((const string*)obj) << "\"}";
        return ss.str();
    }
};

template<>
type_descriptor* get_primitive_descri<std::string>() {
    static type_descriptor_string desc;
    return &desc;
}


// unique_ptr

struct type_descriptor_unique_ptr : type_descriptor {
    type_descriptor* _desc;
    const void* (*_item)(const void*);
    
    template <typename T>
    type_descriptor_unique_ptr(T*) : type_descriptor("std::unique_ptr<>", sizeof(std::unique_ptr<T>)), 
        _desc(type_resolver<T>::get()) {
            _item = [](const void* obj) -> const void* {
                auto p = static_cast<const std::unique_ptr<T>*>(obj);
                return p->get();
            };
        }
    virtual std::string name() const override {
        return std::string("std::unique_ptr<") + _desc->name() + ">";
    }

    virtual std::string dump(const void* obj, int lev) const override {
        std::stringstream ss;

        ss << name() << "{";
        ss << _desc->dump(_item(obj), lev + 1);
        ss << "}";
        return ss.str();
        
    }
};
template <typename T>
struct type_resolver<std::unique_ptr<T>> {
    static type_descriptor* get() {
        static type_descriptor_unique_ptr _desc{(T*)nullptr};
        return &_desc;
    }
};


} // xhb ns
#endif // XHBLIB_REFLECT_VECTOR_H_