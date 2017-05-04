/*
hwloc example
g++ main.cpp  -O2 -o main -std=c++14 `pkg-config --cflags --libs hwloc`
*/

#include <iostream>
using namespace std;

#include <hwloc.h>

static void print_children(hwloc_topology_t topo, hwloc_obj_t obj, int depth) {
	char type[32], attr[1024];

	hwloc_obj_type_snprintf(type, sizeof(type), obj, 0);
	cout << "depth: " << depth << " type: " << type;
	hwloc_obj_attr_snprintf(attr, sizeof(attr), obj, " ", 0);
	cout << " attr: " << attr << endl;

	for (int index = 0; index != obj->arity; ++index) {
		print_children(topo, obj->children[index], depth+1);
	}

}
int main(int argc, const char * argv[]) {
	int topo_depth = 0;
	hwloc_topology_t topo;
	hwloc_obj_t obj;
	char buff[1024] = {};
	int depth = 0;
	size_t size = 0;
	hwloc_cpuset_t cpuset;

	hwloc_topology_init(&topo);
	hwloc_topology_load(topo);

	topo_depth = hwloc_topology_get_depth(topo);
	cout << "hw topology depth: " << topo_depth << endl;

	for (depth = 0; depth < topo_depth; ++depth) {
		cout << "the depth: " << depth;
		for (int index = 0; index != hwloc_get_nbobjs_by_depth(topo, depth); ++index) {
			hwloc_obj_type_snprintf(buff, sizeof(buff), hwloc_get_obj_by_depth(topo, depth, index), 0);
			cout << " index: " << buff;
		}
		cout << endl;
	}

	print_children(topo, hwloc_get_root_obj(topo),0);

	depth = hwloc_get_type_depth(topo, HWLOC_OBJ_PACKAGE);
	if (depth == HWLOC_TYPE_DEPTH_UNKNOWN)
		cout << "packgae is unknown" << endl;
	else
		cout << "package depth: " << depth << endl;
	
	int caches_num = 0;
	size = 0;
	for (obj = hwloc_get_obj_by_type(topo, HWLOC_OBJ_PU, 0); obj; obj = obj->parent) {
		if (obj->type == HWLOC_OBJ_CACHE) {
			caches_num++;
			size += obj->attr->cache.size;
		}
	}
	cout << "logical processor 0 has " << caches_num << " caches,total " << size /1024 << " KB" << endl;

	// 有的硬件并没有core这层,所以使用hwloc_get_type_or_below_depth
	depth = hwloc_get_type_or_below_depth(topo, HWLOC_OBJ_CORE);
	obj = hwloc_get_obj_by_depth(topo, depth, hwloc_get_nbobjs_by_depth(topo, depth) - 1);
	if (obj) {
		cpuset = hwloc_bitmap_dup(obj->cpuset);

		char* str = nullptr;
		hwloc_bitmap_asprintf(&str, cpuset);
		cout << str <<endl;
		free(str);
		// 只开启一个
		hwloc_bitmap_singlify(cpuset);
		hwloc_bitmap_asprintf(&str, cpuset);
		cout << str <<endl;
		free(str);
		int iret = hwloc_set_cpubind(topo, cpuset, 0);
		cout << "hwloc_set_cpubind return " << iret << endl;
		if (iret) {
			char *str = nullptr;
			int error = errno;
			hwloc_bitmap_asprintf(&str, obj->cpuset);
			// osx 10.12 得到函数未实现的错误 =。=
			cout << "bind fail bitmap: " << str << " errno: " << strerror(error) << endl;
			free(str);
		}
		hwloc_bitmap_free(cpuset);
	}

	int n = hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_NUMANODE);
	if (n) {
		// 最后1个
		obj = hwloc_get_obj_by_type(topo, HWLOC_OBJ_NUMANODE, n - 1);
		void *m = hwloc_alloc_membind_nodeset(topo, 1 << 20, obj->nodeset, HWLOC_MEMBIND_BIND,0);
		if (!m) {
			cout << strerror(errno) << endl;
		} 
		hwloc_free(topo, m, 1 << 20);

		// 绑定已分配的内存
		m = malloc(1 << 20);
		if (!m) abort();
		// osx 10.12 返回函数未实现。
		int iret = hwloc_set_area_membind_nodeset(topo, m, 1 << 20, obj->nodeset, HWLOC_MEMBIND_BIND, 0);
		if (iret == -1) {
			cout << strerror(errno) << endl;
		}
		free(m);
	}
	hwloc_topology_destroy(topo);

    return 0;
}

