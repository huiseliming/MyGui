#pragma once
#include <memory>
#include <atomic>
#include "EasyGuiExport.h"

enum EQueueMode
{
    QM_SPSC,
    QM_MPSC,
};

// lock free queue 
template<typename T, EQueueMode QueueMode = EQueueMode::QM_MPSC>
class TQueue
{
    template<EQueueMode QueueMode>
    std::memory_order QueueModeMemoryOrder()
    {
        if constexpr (QueueMode == EQueueMode::QM_MPSC)
            return std::memory_order::relaxed;
        else
            return std::memory_order::acquire;
    }
    TQueue(const TQueue&) = delete;
    TQueue& operator=(const TQueue&) = delete;
public:
    TQueue()
    {
        Node* new_node = new Node();
        _Head.store(new_node, std::memory_order::relaxed);
        _Tail.store(new_node, std::memory_order::relaxed);
    }

    ~TQueue()
    {
        while (_Tail.load(std::memory_order::relaxed) != nullptr)
        {
            Node* node = _Tail.load(std::memory_order::relaxed);
            _Tail.store(node->_NextNode, std::memory_order::relaxed);
            delete node;
        }
    }

    bool Dequeue(T& out_element)
    {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* popped = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        if (popped == nullptr) return false;
        out_element = std::move(popped->_Element);

        Node* old_tail = tail;
        _Tail.store(popped, std::memory_order::relaxed);
        delete old_tail;
        return true;
    }

    bool Enqueue(const T& element)
    {
        Node* new_node = new Node(element);
        if (new_node == nullptr) return false;
        Node* old_head;
        if (QueueMode == EQueueMode::QM_MPSC)
        {
            old_head = _Head.exchange(new_node);
            old_head->_NextNode.exchange(new_node);
        }
        else
        {
            old_head = _Head;
            _Head.store(new_node, std::memory_order::relaxed);
            old_head->_NextNode.store(new_node, std::memory_order::release);
        }
        return true;
    }

    bool Enqueue(T&& element)
    {
        Node* new_node = new Node(std::move(element));
        if (new_node == nullptr) return false;
        Node* old_head;
        if (QueueMode == EQueueMode::QM_MPSC)
        {
            old_head = _Head.exchange(new_node);
            old_head->_NextNode.exchange(new_node);
        }
        else
        {
            old_head = _Head;
            _Head.store(new_node, std::memory_order::relaxed);
            old_head->_NextNode.store(new_node, std::memory_order::release);
        }
        return true;
    }


    void Empty()
    {
        while (Pop());
    }

    bool IsEmpty() const
    {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* tail_next_node = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        return tail_next_node == nullptr; 
    }

    bool Peek(T& OutElement) const
    {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* tail_next_node = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        if (tail_next_node == nullptr)
        {
            return false;
        }
        OutElement = tail_next_node->Element;
        return true;
    }

    T* Peek()
    {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* tail_next_node = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        if (tail_next_node == nullptr)
        {
            return nullptr;
        }

        return &tail_next_node->Element;
    }

    const T* Peek() const
    {
        return const_cast<TQueue*>(this)->Peek();
    }

    bool Pop()
    {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* popped = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        if (popped == nullptr) return false;
        Node* old_tail = tail;
        _Tail.store(popped);
        delete old_tail;
        return true;
    }

    bool MoreThanOne() {
        Node* tail = _Tail.load(std::memory_order::relaxed);
        Node* first_node = tail->_NextNode.load(QueueModeMemoryOrder<QueueMode>());
        if (first_node)
        {
            if (first_node->_NextNode.load(std::memory_order::relaxed)) {
                return true;
            }
        }
        return false;
    }

private:
    struct Node
    {
        Node() : _NextNode(nullptr) { }
        explicit Node(const T& element) : _NextNode(nullptr), _Element(element) { }
        explicit Node(T&& element) : _NextNode(nullptr), _Element(std::move(element)) { }
        std::atomic<Node*> _NextNode;
        T _Element;
    };
    std::atomic<Node*> _Head;
    std::atomic<Node*> _Tail;
};


