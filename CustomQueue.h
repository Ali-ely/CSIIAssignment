#ifndef SIMWORLD_CUSTOM_QUEUE_H
#define SIMWORLD_CUSTOM_QUEUE_H

#include <cstddef>
#include <utility>

// An owning linked FIFO queue with O(1) insertion/removal. Const traversal
// allows the menu to inspect pending events without consuming them (F6/F8).
template <typename T>
class CustomQueue {
    struct Node {
        T value;
        Node* next = nullptr;
        explicit Node(const T& item) : value(item) {}
        explicit Node(T&& item) : value(std::move(item)) {}
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;

    void link(Node* node) noexcept {
        if (tail_) tail_->next = node;
        else head_ = node;
        tail_ = node;
        ++size_;
    }

public:
    CustomQueue() = default;
    CustomQueue(const CustomQueue& other) {
        try {
            for (const Node* node = other.head_; node; node = node->next)
                enqueue(node->value);
        } catch (...) {
            clear();
            throw;
        }
    }
    CustomQueue(CustomQueue&& other) noexcept { swap(other); }
    CustomQueue& operator=(CustomQueue other) {
        swap(other);
        return *this;
    }
    ~CustomQueue() { clear(); }

    void swap(CustomQueue& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }
    void enqueue(const T& value) { link(new Node(value)); }
    void enqueue(T&& value) { link(new Node(std::move(value))); }
    bool dequeue(T& result) {
        if (!head_) return false;
        result = std::move(head_->value);
        Node* old = head_;
        head_ = old->next;
        delete old;
        --size_;
        if (!head_) tail_ = nullptr;
        return true;
    }
    const T* front() const { return head_ ? &head_->value : nullptr; }
    bool empty() const { return head_ == nullptr; }
    std::size_t size() const { return size_; }
    template <typename Visitor>
    void forEach(Visitor visitor) const {
        for (const Node* node = head_; node; node = node->next)
            visitor(node->value);
    }
    void clear() noexcept {
        while (head_) {
            Node* old = head_;
            head_ = old->next;
            delete old;
        }
        tail_ = nullptr;
        size_ = 0;
    }
};

#endif
