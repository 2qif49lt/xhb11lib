// g++ rueue_test.cpp  -o test -std=c++14 -DRUEUE_UNIT_TEST

#ifdef RUEUE_UNIT_TEST

#include "rueue.h"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <utility>
#include <thread>
#include <chrono>
#include <type_traits>

using std::string;
using std::vector;

void test_construct(){
    rueue<int> a;
    rueue<int> b(5);
    string Ivanka = "Ivanka";
    rueue<string> c(5,Ivanka);
    
    assert(b.size() == 5 && c.size() == 5);
    assert(a.empty() == true && b.empty() == false);
    auto t = c[4];
    
    assert(t == Ivanka && c.front() == Ivanka && c.back() == Ivanka && c.at(2) == Ivanka);
    
    vector<int> veca;
    int lmd_n = 0;
    std::generate_n(std::back_inserter(veca),10,[&lmd_n](){ return lmd_n++;});
    
    rueue<int> d(veca.begin(),veca.end());
    assert(d.size() == veca.size());
    
    rueue<string> e(c);
    assert(e.size() == c.size() && e[1] == c[1]);
    
    rueue<string> f(std::move(e));
    assert(f.size() == c.size() && f[1] == c[1] && e.size() == 0);
    
    
    a = b;
    assert(a.size() == b.size() && a[1] == b[1]);
    
    a = std::move(d);
    assert(a.size() == 10 && a[1] == 1 && d.size() == 0);
    
    a.swap(b);
    assert(a.size() == 5 && b.size() == veca.size());
}

void test_member(){
    rueue<int> ir;
    vector<int> vec;
    int n = 0;
    std::generate_n(std::back_inserter(vec), 10,[&n](){return n++;});
    
    ir.assign(5, 5);
    assert(ir.size() == 5 && ir[1] == 5);
    
    ir.assign(vec.begin(), vec.end());
    assert(ir.size() == 10&&ir[1] == 1);
    
    
    rueue<string> r;
    r.push_back("Ivanka");
    assert(r.size() == 1 && r[0] == "Ivanka" && r[0] == r.back());
    
    r.push_back("Kardashian");
    assert(r.size() == 2 && r[1] == "Kardashian" && r[1] == r.back());
    
    r.push_front("Hathaway");
    assert(r.size() == 3&& r[0] == "Hathaway" && r[0] == r.front());
    
    r.pop_back();
    assert(r.size() == 2 && r.back() == "Ivanka");
    
    r.pop_front();
    assert(r.size() == 1 && r.back() == "Ivanka" && r.front() == r.back());
    
    r.emplace_back("Kardashian");
    r.emplace_front("Hathaway");
    assert(r.size() == 3 && r.front() == "Hathaway" && r.back() == "Kardashian");
    
    
    
    r.clear();
    assert(r.size() == 0);
    
    
    /*
    int c = 0;
    while(1){
        std::cout<<c++<<std::endl;
        if(c % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for(int count = 0; count != 100000; ++count){
            r.push_front("Ivanka");
        }
        for(int count = 0; count != 100000; ++count){
            r.pop_back();
        }
        assert(r.size() == 0 );
    }
    */


}

void test_iterator(){
    rueue<int> r(10,1);
    
    auto iter = r.begin();
    assert(*iter == 1);
    
    ++iter;
    *iter = 0;
    assert(r[1] == 0);
    
    int n = 0;
    for_each(r.begin(),r.end(),[&n](int& i){i = n++;});
    assert(r.back() == 9);
   
    std::generate_n(std::back_inserter(r), 10, [&n](){return n++;});
    assert(r.size() == 20 && r[15] == 15);
    
    auto citer = r.cbegin();
    assert(*citer == 0);
    
    auto b = std::is_same<const int, rueue<int>::const_iterator::value_type>::value;
    assert(b);

    b = std::is_const<rueue<int>::const_iterator::value_type>::value;
    assert(b);
    
    b = std::is_const<rueue<int>::iterator::value_type>::value;
    assert(b == false);
    
    int idx = 0;
    for(auto it = r.begin(); it != r.end(); ++it,++idx){
        assert(*it == r[idx] && *it == idx);
    }
    
    auto fit = find(r.begin(),r.end(),10);
    assert(fit != r.end());
    
    fit = r.erase(fit);
    assert(*fit == 11 && r.size() == 19);
    assert(r.end() == find(r.begin(),r.end(),10));
    
    r.push_front(10);
    auto eit = r.erase(r.begin(), r.end());
    assert(eit == r.end() && r.size() == 0);
    
    auto iit = r.insert(r.begin(), 1);
    assert(*iit == 1);
    iit = r.insert(iit, 2);
    assert(*iit == 2 && r.back() == 1);
    
    
    auto ieit = r.insert(r.end(), 3);
    assert(r.back() == 3 && *ieit == 3);
    
    
    vector<int> vec;
    n = 0;
    std::generate_n(std::back_inserter(vec), 10, [&n](){return n++;});
    
    iit = r.insert(r.begin() + 1, 3, 5);
    assert(*iit == 5 && *(iit+1) == 5 && *(iit + 2) == 5);
    assert(r.size() == 6);
    std::for_each(r.begin(),r.end(),[](int i){std::cout<<" "<<i;});
    std::cout<<std::endl;
    
    iit = r.insert(r.end() - 2, vec.begin(), vec.end());
    assert(r.size() == 6 + 10);
    std::for_each(r.begin(),r.end(),[](int i){std::cout<<" "<<i;});
    std::cout<<std::endl;
}
int main(){
    test_construct();
    test_member();
    test_iterator();
}
#endif // RUEUE_UNIT_TEST
