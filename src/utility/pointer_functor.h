#ifndef XHBLIB_POINT_FUNCTOR_H_
#define XHBLIB_POINT_FUNCTOR_H_

// 提供一些对指针、智能指针类型进行直接值比较，哈希的functor
/*
    int i = 0, j = 1;
    int* pi = &i;
    int* pj = &j;
    int* pn = nullptr;
    
    auto inth = pointer_hasher<int*>();
    cout << inth(pi) << " " << inth(pj) << " " << inth(pn) <<endl;
    
    string stra = "hello",strb = "world";
    string* pstra = &stra;
    string* pstrb = &strb;
    string* pstrn = nullptr;
    auto strh = pointer_hasher<string*>();
    cout << strh(pstra) << " " << strh(pstrb) << " " << strh(pstrn) <<endl;
    
    auto smta = make_shared<string>("hello");
    auto smtb = make_shared<string>("world");
    decltype(smta) smtn;
    auto smth = pointer_hasher<decltype(smta)>();
    cout << smth(smta) << " " << smth(smtb) << " " << smth(smtn) << endl;
    
    
    
    map<shared_ptr<string>,int,pointer_lesser<shared_ptr<string>>> m;
    
    m[smta] = 1;
    m[smtb] = 2;
    m[smtn] = 0;
    
    cout << m.find(smta)->second <<endl;
    
    
    unordered_map<shared_ptr<string>,
        int,
        pointer_hasher<shared_ptr<string>>,
        pointer_equaler<shared_ptr<string>>> um;
    
    um[smta] = 1;
    um[smtb] = 2;
    um[smtn] = 0;
   
    cout << um.find(smtb)->second <<endl;

*/

#include <memory>
#include <functional>
#include <utility>

template<typename P, 
    typename H = std::hash<typename std::pointer_traits<P>::element_type>>
struct pointer_hasher {
    H _hash;
    pointer_hasher(H h = H()) : _hash(std::move(h)) {}
    size_t operator()(const P& p) const {
        if (p) return _hash(*p);
        return 0;
    }
};

template<typename P,
    typename L = std::less<typename std::pointer_traits<P>::element_type>>
struct pointer_lesser {
    L _less;
    pointer_lesser(L l = L()) : _less(std::move(l)) {}
    bool operator()(const P& lhs,const P& rhs) const {
        if ((bool)lhs && (bool)rhs) return _less(*lhs,*rhs);
        if(!lhs) return (bool)rhs;
        return false;
    }
};

template<typename P,
    typename E = std::equal_to<typename std::pointer_traits<P>::element_type>>
struct pointer_equaler {
    E _equal;
    pointer_equaler(E e = E()) : _equal(std::move(e)) {}
    bool operator()(const P& lhs,const P& rhs) const {
        if((bool)lhs ^ (bool)rhs) return false;
        if(!lhs) return true;
        return _equal(*lhs,*rhs);
    }
};

#endif // XHBLIB_POINT_FUNCTOR_H_
