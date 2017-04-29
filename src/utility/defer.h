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
 */
 #include<functional>
namespace xhb
{
 
 class defer_
 {
 public:
     explicit defer_(std::function<void()> onexit) : m_onexit(onexit),m_bdismiss(false){}
     ~defer_()
     {
         if(!m_bdismiss)
             m_onexit();
     }
     void dismiss(){m_bdismiss = true;}
 private:
     std::function<void()> m_onexit;
     bool m_bdismiss;
 
 private:
     defer_(const defer_& rhs);
     defer_& operator= (const defer_& rhs);
 };
 
#define defer_name_cat(name,line) name##line
#define defer_name(name,line) defer_name_cat(name,line)
#define defer(fn) defer_ defer_name(defer_name_,__LINE__) (fn)


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