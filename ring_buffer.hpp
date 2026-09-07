#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <atomic>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <iostream>

template <typename T, size_t capacity, typename Allocator = std::allocator<T>>
class SPSCQueue
{
public:
    static_assert((capacity & (capacity - 1)) == 0, "capacity must be power of 2");
    SPSCQueue();
    bool push(const T &item);
    bool pop(T &item);
    size_t size();
    uint8_t *internal_buffer();
    size_t internal_buffer_size();
    bool empty();

private:
    std::vector<T, Allocator> m_buffer;
    alignas(64) std::atomic<size_t> m_head;
    alignas(64) std::atomic<size_t> m_tail;
};

template <typename T, size_t capacity, typename Allocator>
SPSCQueue<T, capacity, Allocator>::SPSCQueue()
{
    m_buffer.resize(capacity);
    m_head = 0;
    m_tail = 0;
}

template <typename T, size_t capacity, typename Allocator>
bool SPSCQueue<T, capacity, Allocator>::push(const T &item)
{
    size_t current_head = m_head.load(std::memory_order_relaxed);
    size_t next_head = (current_head + 1) & (capacity - 1);

    // 生产者一旦看到更新后的尾指针，那么数据一定已被消费者读走（因为消费中是先读取再relase语义store）
    if (next_head == m_tail.load(std::memory_order_acquire))
    {
        return false;
    }

    m_buffer[current_head] = item;
    // release保证，实际写入后才更新了头指针
    // 即使在写入后、更新头指针前，另一个线程正在读取，那么最糟的情况也只是以为没有数据可读
    m_head.store(next_head, std::memory_order_release);
    return true;
}

template <typename T, size_t capacity, typename Allocator>
bool SPSCQueue<T, capacity, Allocator>::pop(T &item)
{

    size_t current_tail = m_tail.load(std::memory_order_relaxed);

    // 消费者一旦看到更新后的头指针，那么数据一定已被生产者写入（因为生产者中是先写入再relase语义store）
    if (current_tail == m_head.load(std::memory_order_acquire))
    {
        return false;
    }

    item = m_buffer[current_tail];
    // release保证，实际读取后才更新了尾指针
    // 即使在读取后、更新尾指针前，另一个线程正在写入，那么最糟的情况也只是以为空间还未释放
    m_tail.store((current_tail + 1) & (capacity - 1), std::memory_order_release);
    return true;
}

template <typename T, size_t capacity, typename Allocator>
size_t SPSCQueue<T, capacity, Allocator>::size()
{
    size_t current_head = m_head.load(std::memory_order_acquire);
    size_t current_tail = m_tail.load(std::memory_order_acquire);

    if (current_head >= current_tail)
    {
        return current_head - current_tail;
    }
    else
    {
        return capacity + current_head - current_tail;
    }
}

template <typename T, size_t capacity, typename Allocator>
uint8_t *SPSCQueue<T, capacity, Allocator>::internal_buffer()
{
    return (uint8_t *)m_buffer.data();
}

template <typename T, size_t capacity, typename Allocator>
size_t SPSCQueue<T, capacity, Allocator>::internal_buffer_size()
{
    return m_buffer.size() * sizeof(T);
}

template <typename T, size_t capacity, typename Allocator>
bool SPSCQueue<T, capacity, Allocator>::empty()
{
    size_t current_head = m_head.load(std::memory_order_acquire);
    size_t current_tail = m_tail.load(std::memory_order_acquire);

    if (current_head == current_tail)
        return true;

    return false;
}

#endif