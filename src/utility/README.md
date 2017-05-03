####hwloc 
1. 安装hwloc (<https://www.open-mpi.org/software/hwloc/v1.11/>)
2. 编译安装
```
./configure // 或 ./configure --prefix=`pwd`/out
./make
./make install
```
3. 测试`lstopo`
4. 测试API
```
#include <hwloc.h>
int main(int argc, const char * argv[]) {

	hwloc_topology_t topology;
	int nbcores;

	hwloc_topology_init(&topology);  // initialization
	hwloc_topology_load(topology);   // actual detection

	nbcores = hwloc_g_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
	printf("%d cores\n", nbcores);

	hwloc_topology_destroy(topology);
    return 0;
}
// g++ main.cpp -O2 -o main -std=c++11 `pkg-config --cflags --libs hwloc`
// 如果是提示找不到文件需要 将pc文件所在目录加入PKG_CONFIG_PATH:export PKG_CONFIG_PATH:...`
```
