#ifndef SIMWORLD_CUSTOM_STACK_H
#define SIMWORLD_CUSTOM_STACK_H

#include <cstddef>
#include <utility>

// An owning linked LIFO stack. Snapshots are values, so copying the stack also
// copies its worlds. No standard container supplies the stack behavior (F8).
template <typename T>
class CustomStack {
    struct Node {
        T value;
        Node* next;
        Node(const T& item, Node* link) : value(item), next(link) {}
        Node(T&& item, Node* link) : value(std::move(item)), next(link) {}
    };
    Node* top_ = nullptr;
    std::size_t size_ = 0;

public:
    CustomStack() = default;
    CustomStack(const CustomStack& other) {
        Node** nextLink = &top_;
        try {
            for (const Node* source = other.top_; source; source = source->next) {
                *nextLink = new Node(source->value, nullptr);
                nextLink = &((*nextLink)->next);
                ++size_;
            }
        } catch (...) {
            clear();
            throw;
        }
    }
    CustomStack(CustomStack&& other) noexcept { swap(other); }
    CustomStack& operator=(CustomStack other) {
        swap(other);
        return *this;
    }
    ~CustomStack() { clear(); }

    void swap(CustomStack& other) noexcept {
        std::swap(top_, other.top_);
        std::swap(size_, other.size_);
    }
    void push(const T& value) {
        top_ = new Node(value, top_);
        ++size_;
    }
    void push(T&& value) {
        top_ = new Node(std::move(value), top_);
        ++size_;
    }
    bool pop(T& result) {
        if (!top_) return false;
        result = std::move(top_->value);
        Node* old = top_;
        top_ = old->next;
        delete old;
        --size_;
        return true;
    }
    const T* top() const { return top_ ? &top_->value : nullptr; }
    bool empty() const { return top_ == nullptr; }
    std::size_t size() const { return size_; }
    void clear() noexcept {
        while (top_) {
            Node* old = top_;
            top_ = old->next;
            delete old;
        }
        size_ = 0;
    }
};

#endif
