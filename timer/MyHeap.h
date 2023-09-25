#ifndef TIMER_HEAP_H
#define TIMER_HEAP_H

#include <vector>
#include <algorithm>

// 默认是小根堆
template<typename T, typename cmp = std::less<T>>
class MyHeap {
public:
    MyHeap() = default;

    void add(T t);
    void del(T t);
    void adjust();
    void pop() { del(heap_[0]); }
    void empty() { return heap_.empty(); }

private:
    void heapifyUp(int k);
    void heapifyDown(int k);

    std::vector<T> heap_;
};

template <typename T, typename cmp>
inline void MyHeap<T, cmp>::add(T t) {
    heap_.push_back(t);
    heapifyUp(heap_.size() - 1);
}

template <typename T, typename cmp>
inline void MyHeap<T, cmp>::del(T t) {
    // 找到对应的元素，和队尾元素进行交换，删除该元素
    auto it = std::find(heap_.begin(), heap_.end(), t);
    std::swap(*it, heap_.back());
    heap_.pop_back();
    // 调整堆
    heapifyDown(it - heap_.begin());
}

template <typename T, typename cmp>
void MyHeap<T, cmp>::adjust() {
    for (int i = heap_.size() / 2; i >= 0; --i) {
        heapifyDown(i);
    }
}

template <typename T, typename cmp>
void MyHeap<T, cmp>::heapifyUp(int k) {
    int parent = (k - 1) / 2;
    while (parent >= 0) {
        if (cmp()(heap_[k], heap_[parent])) { // 如果子节点小于父节点
            std::swap(heap_[k], heap_[parent]);
        } else { // 无法上浮
            break;
        }
        k = parent;
        parent = (k - 1) / 2;
    }
}

template <typename T, typename cmp>
void MyHeap<T, cmp>::heapifyDown(int k) {
    while (2 * k + 1 < heap_.size()) {
        int left = 2 * k + 1;
        int right = 2 * k + 2;

        int minNum = k;
        if (left < heap_.size() && cmp()(heap_[left], heap_[minNum])) { minNum = left; }
        if (right < heap_.size() && cmp()(heap_[right], heap_[minNum])) { minNum = right; }

        // 无法下沉，已经调整完毕
        if (k == minNum) break;
        std::swap(heap_[k], heap_[minNum]);
        k = minNum;
    }
}


#endif //TIMER_HEAP_H