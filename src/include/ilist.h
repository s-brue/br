// MIT License
//
// Copyright (c) 2024 Sergio Pérez Camacho
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BR_ILIST_H_
#define BR_ILIST_H_

#include <mutex>

namespace br {

namespace detail_ { struct void_mutex { void lock() noexcept {} void unlock() noexcept {} }; }

template <typename T, typename MUTEX_LOCK=detail_::void_mutex>
class ilist {
    friend class node;

public:
    class iterator;

    class node {
        friend class ilist;
        friend class iterator;

    public:
        T* link_node_before(node* node) noexcept
        {
            return parent_list_ ? parent_list_->link_node_before(this, node) : nullptr;
        }

        T* link_node_after(node* node) noexcept
        {
            return parent_list_ ? parent_list_->link_node_after(this, node) : nullptr;
        }

        void unlink() noexcept
        {
            if (parent_list_) parent_list_->unlink_node(this);
        }

        virtual ~node() noexcept { unlink(); }

        const T* next() const noexcept { return static_cast<const T*>(next_); }
        const T* prev() const noexcept { return static_cast<const T*>(prev_); }

    private:
        node*  prev_{nullptr};
        node*  next_{nullptr};
        ilist* parent_list_{nullptr};
    };

    class iterator {
    public:
        explicit iterator(node* current) noexcept : current_(current)
        {
        }

        iterator& operator++() noexcept
        {
            current_ = current_->next_;
            return *this;
        }

        iterator& operator--() noexcept
        {
            current_ = current_->prev_;
            return *this;
        }

        [[nodiscard ]] iterator operator++(int) const noexcept { return {current_->next()}; }
        [[nodiscard ]] iterator operator--(int) const noexcept { return {current_->prev()}; }

        bool operator==(const iterator& o) const noexcept { return current_ == o.current_; }

        [[nodiscard]] T& operator*() const noexcept { return *static_cast<T*>(current_); }

    private:
        node* current_;
    };

    constexpr iterator begin() noexcept { return iterator(head_.next_); }
    constexpr iterator end() noexcept { return iterator(&tail_); }

    ilist() noexcept: n_entries_(0)
    {
        head_.prev_        = nullptr;
        head_.next_        = &tail_;
        tail_.prev_        = &head_;
        tail_.next_        = nullptr;
        head_.parent_list_ = this;
        tail_.parent_list_ = this;
    }

    void push_front(node* node) noexcept
    {
        head_.link_node_after(node);
    }

    void push_back(node* node) noexcept
    {
        tail_.link_node_before(node);
    }

    T* pop_front()
    {
        std::lock_guard<MUTEX_LOCK> l(mutex_lck_);
        if (head_.next_ == &tail_) return nullptr;
        auto* node = head_.next_;
        non_locking_unlink_node(node);
        return static_cast<T*>(node);
    }

    T* pop_back()
    {
        std::lock_guard<MUTEX_LOCK> l(mutex_lck_);
        if (tail_.prev_ == &head_) return nullptr;
        auto* node = tail_.prev_;
        non_locking_unlink_node(node);
        return static_cast<T*>(node);
    }

    [[nodiscard]] T* front() const noexcept
    {
        return static_cast<T*>(head_.next_);
    }

    [[nodiscard]] T* back() const noexcept
    {
        return static_cast<T*>(tail_.prev_);
    }

    [[nodiscard]] size_t size() const noexcept { return n_entries_; }
    [[nodiscard]] bool   empty() const noexcept { return n_entries_ == 0; }

    void clear()
    {
        std::lock_guard<MUTEX_LOCK> l(mutex_lck_);
        n_entries_ = 0;

        auto* node = head_.next_;
        while (node != &tail_) {
            node->parent_list_ = nullptr;
            node = node->next_;
        }

        head_.next_ = &tail_;
        tail_.prev_ = &head_;
    }

private:
    void unlink_node(node* node)
    {
        std::lock_guard<MUTEX_LOCK> lock(mutex_lck_);
        non_locking_unlink_node(node);
    }

    void non_locking_unlink_node(node* node)
    {
        if (node->prev_) node->prev_->next_ = node->next_;
        if (node->next_) node->next_->prev_ = node->prev_;
        --n_entries_;

        node->parent_list_ = nullptr;
        node->prev_        = nullptr;
        node->next_        = nullptr;
    }

    T* link_node_before(node* current, node* node)
    {
        if (node->parent_list_) return nullptr;

        std::lock_guard<MUTEX_LOCK> lock(mutex_lck_);

        node->parent_list_ = current->parent_list_;
        node->next_        = current;
        node->prev_        = current->prev_;

        ++n_entries_;
        current->prev_->next_ = node;
        current->prev_        = node;
        return static_cast<T*>(node);
    }

    T* link_node_after(node* current, node* node)
    {
        if (node->parent_list_) return nullptr;

        std::lock_guard<MUTEX_LOCK> lock(mutex_lck_);

        node->parent_list_ = current->parent_list_;
        node->prev_        = current;
        node->next_        = current->next_;

        ++n_entries_;
        current->next_->prev_ = node;
        current->next_        = node;
        return static_cast<T*>(node);
    }

    size_t n_entries_;
    node   head_;
    node   tail_;

    MUTEX_LOCK mutex_lck_;
};
} // namespace br

#endif // BR_ILIST_H_
