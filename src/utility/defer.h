#ifndef XHBLIB_DEFER_H_
#define XHBLIB_DEFER_H_

 /*
  void testdefer()
 {
     FILE* pf = fopen("d:/1.txt","r");
     defer([&](){if(pf)fclose(pf);});
     
     char chbuff[10240] = {};
     if(pf)
     {
         int iread = fread(chbuff,sizeof(char),sizeof(chbuff),pf);
        if(iread > 0)
         cout<<chbuff<<endl;
     }
 }


 int main(int argc, const char * argv[]) {
    
    int i = 0;
    {
        defer([&i](){i++; cout << "a " << i <<endl;});
        cout << i <<endl;
        defer([&i](){i++; cout << "b " << i <<endl;});
        
    }
    
    return 0;
}

 */
#include <type_traits>
#include <utility>

namespace xhb
{
 template<typename F>
 class defer_impl
 {
 public:
     static_assert(std::is_nothrow_move_constructible<F>::value, "F must be noexcept");

    defer_impl(F&& func) noexcept : _f(std::move(func)) {}
    defer_impl(defer_impl&& d) noexcept : _f(std::move(d._f)),_bdismiss(d._bdismiss) {
        d._bdismiss = true;
    }
    defer_impl(const defer_impl&) = delete;
    defer_impl& operator=(const defer_impl&) = delete;
    defer_impl& operator=(defer_impl&& other) noexcept {
        if (this != &other) {
            this->~defer_impl();
            new (this) defer_impl(std::move(other));
        }
        return *this;
    }
     ~defer_impl() {
         if(!_bdismiss)
             _f();
     }
     void dismiss() {_bdismiss = true;}
 private:
     F _f;
     bool _bdismiss = false;
 };
 
 template<typename F>
 inline defer_impl<F> get_defer(F&& func) {
     return defer_impl<F>(std::forward<F>(func));
 }

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(prefix) PP_CAT(prefix, __COUNTER__)

#define defer(fn)  auto UNIQUE_NAME(defer_) = xhb::get_defer(fn)

 }
#endif //XHBLIB_DEFER_H_

 /////////
 // or

 /*
 #include <memory>
#include <stdarg.h>
typedef shared_ptr<void> defer_ptr;

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(prefix) PP_CAT(prefix, __COUNTER__)

#define defer(func,...) \
    defer_ptr UNIQUE_NAME(defer_)(nullptr,[&](...){func(__VA_ARGS__);})

void testshareptr()
{
    defer_ptr _(nullptr,[&](...){cout<<"function end"<<endl;});
    cout<<"function begin"<<endl;
    {
        defer_ptr _(nullptr,[&](...){cout<<"scope end"<<endl;});
        cout<<"scope begin"<<endl;
    }
    cout<<"function will end"<<endl;;
}
int main(_In_ int _Argc, _In_reads_(_Argc) _Pre_z_ char ** _Argv, _In_z_ char ** _Env)
{
    int i = 0;
    {
        defer([](){cout<<"main last"<<endl;});
        defer([&](){cout<<i<<endl;});
        defer([&](int j){cout<<i<<" "<<j<<endl;},10);
        defer([&](int j,int* pi,const string& s){cout<<i<<" "<<j<<" "<<*pi<<" "<<s<<endl;},10,&i,"abc");
        testshareptr();
        ++i;
    }
    return 0;
}
 */