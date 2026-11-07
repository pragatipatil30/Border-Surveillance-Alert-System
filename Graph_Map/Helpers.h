// ============================================================================
//  person3/Helpers.h
//  Small building blocks used inside Person 3's code.
//
//  SimpleStack and SimpleQueue are minimal array-based versions so that
//  Person 3's folder compiles on its own, without depending on Person 2's
//  Stack.h / Queue.h. They are used for:
//      - iterative tree traversals (stack)
//      - iterative DFS               (stack)
//      - BFS and level-order         (queue)
//  After integration they can optionally be swapped for Person 2's classes.
//
//  EventCallback is the "visualizer hook": a structure reports what it is
//  doing as a text message and the Qt Structure Lab can display it. The data
//  structures never know whether anyone is listening.
// ============================================================================
#ifndef P3_HELPERS_H
#define P3_HELPERS_H

#include <functional>
#include <string>

using EventCallback = std::function<void(const std::string&)>;

inline void emitEvent(const EventCallback& cb, const std::string& message)
{
    if (cb) cb(message);
}

// ---------------------------------------------------------------------------
//  SimpleStack<T>: growable array stack.  push / pop / peek are O(1) amortized.
//  The caller must check empty() before pop() or peek().
// ---------------------------------------------------------------------------
template <typename T>
class SimpleStack
{
private:
    T*  data_;
    int capacity_;
    int top_;

    void grow()
    {
        int newCapacity = capacity_ * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < top_; i++) bigger[i] = data_[i];
        delete[] data_;
        data_ = bigger;
        capacity_ = newCapacity;
    }

public:
    explicit SimpleStack(int initialCapacity = 16)
        : data_(new T[initialCapacity > 0 ? initialCapacity : 16]),
          capacity_(initialCapacity > 0 ? initialCapacity : 16),
          top_(0) {}

    ~SimpleStack() { delete[] data_; }

    SimpleStack(const SimpleStack&)            = delete;
    SimpleStack& operator=(const SimpleStack&) = delete;

    void push(const T& value)
    {
        if (top_ == capacity_) grow();
        data_[top_++] = value;
    }

    T    pop()         { return data_[--top_]; }
    T&   peek()        { return data_[top_ - 1]; }
    bool empty() const { return top_ == 0; }
    int  size()  const { return top_; }
};

// ---------------------------------------------------------------------------
//  SimpleQueue<T>: growable circular-array queue.
//  enqueue / dequeue are O(1) amortized.
//  The caller must check empty() before dequeue().
// ---------------------------------------------------------------------------
template <typename T>
class SimpleQueue
{
private:
    T*  data_;
    int capacity_;
    int front_;
    int count_;

    void grow()
    {
        int newCapacity = capacity_ * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < count_; i++)
            bigger[i] = data_[(front_ + i) % capacity_];
        delete[] data_;
        data_ = bigger;
        capacity_ = newCapacity;
        front_ = 0;
    }

public:
    explicit SimpleQueue(int initialCapacity = 16)
        : data_(new T[initialCapacity > 0 ? initialCapacity : 16]),
          capacity_(initialCapacity > 0 ? initialCapacity : 16),
          front_(0), count_(0) {}

    ~SimpleQueue() { delete[] data_; }

    SimpleQueue(const SimpleQueue&)            = delete;
    SimpleQueue& operator=(const SimpleQueue&) = delete;

    void enqueue(const T& value)
    {
        if (count_ == capacity_) grow();
        data_[(front_ + count_) % capacity_] = value;
        count_++;
    }

    T dequeue()
    {
        T value = data_[front_];
        front_ = (front_ + 1) % capacity_;
        count_--;
        return value;
    }

    bool empty() const { return count_ == 0; }
    int  size()  const { return count_; }
};

#endif // P3_HELPERS_H
