#ifndef XHBLIB_LINK_QUEUE_H_
#define XHBLIB_LINK_QUEUE_H_

// lueue 区别std:deque在于:deque内部用数组进行索引节点并支持随机访问插入等操作，而lueue用链表进行管理节点，只支持队列首尾的操作。
// 主要的优点是可以配置节点元素大小、剔除了deque的部分功能保证所有操作都是O(1)、内存分配平滑。
// 相比rueue，lueue优点体现在分配内存时波动平滑。

template<typename T,size_t NODE_ITEM_SIZE = 128>
class lueue {
public:

};
#endif // XHBLIB_LINK_QUEUE_H_