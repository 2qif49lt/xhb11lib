#ifdef XHBLIB_ALGO_UNITS_TEST

#include <cassert>
#include <iostream>
using namespace std;

#include "src/utility/algo_utils.h"


void funca() { cout << "funca" << endl;}
int funcb() { cout << "funcb" << endl;  return 1; }
int* funcc() { cout << "funcc" << endl; return nullptr; }

struct stta {
    operator bool() const { return true; }
    ~stta() { cout << "stta" << endl; }
};

int main()
{	
    xhb::repeat_if(10, [](){ cout << 0 << endl; });
	xhb::repeat_if(10, [](int i){ cout << i << endl; }, 10);

    int k = 0;
	xhb::repeat_if(10, [](int j, int &k){ 
		static int i = 0; 
		i++;
        ++k;
		cout << j << endl;  
		if (i > 5) 
			return false; 
		return true;
	}, 11, k);
    assert(k == 6);

    xhb::repeat_if(10, funca);
    xhb::repeat_if(10, funcb);
    xhb::repeat_if(10, funcc);

    xhb::repeat_if(10, [](){ return stta(); });

	return 0;
}

#endif
