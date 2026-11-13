#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace border_api::dsa {

struct PriorityItem {
    int value;
    int priority;
    std::uint64_t sequence;
};

// Binary max-heap for the DSA Lab. Equal priorities retain insertion order.
class PriorityQueue {
public:
    void insert(int value, int priority)
    {
        heap_.push_back({value, priority, nextSequence_++});
        siftUp(heap_.size() - 1);
    }

    bool empty() const { return heap_.empty(); }
    std::size_t size() const { return heap_.size(); }
    const PriorityItem* peek() const { return heap_.empty() ? nullptr : &heap_.front(); }

    bool extract(PriorityItem& item)
    {
        if (heap_.empty()) return false;
        item = heap_.front();
        if (heap_.size() == 1) {
            heap_.pop_back();
            return true;
        }
        heap_.front() = heap_.back();
        heap_.pop_back();
        siftDown(0);
        return true;
    }

    const std::vector<PriorityItem>& exportHeap() const { return heap_; }

private:
    static bool higher(const PriorityItem& a, const PriorityItem& b)
    {
        return a.priority > b.priority ||
               (a.priority == b.priority && a.sequence < b.sequence);
    }

    void siftUp(std::size_t index)
    {
        while (index > 0) {
            const std::size_t parent = (index - 1) / 2;
            if (!higher(heap_[index], heap_[parent])) break;
            std::swap(heap_[index], heap_[parent]);
            index = parent;
        }
    }

    void siftDown(std::size_t index)
    {
        while (true) {
            const std::size_t left = index * 2 + 1;
            const std::size_t right = left + 1;
            std::size_t best = index;
            if (left < heap_.size() && higher(heap_[left], heap_[best])) best = left;
            if (right < heap_.size() && higher(heap_[right], heap_[best])) best = right;
            if (best == index) break;
            std::swap(heap_[index], heap_[best]);
            index = best;
        }
    }

    std::vector<PriorityItem> heap_;
    std::uint64_t nextSequence_ = 0;
};

} // namespace border_api::dsa
