#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    WeakPtr() : ptr_(nullptr), block_(nullptr){};

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        SafeWeakIncrement();
    }
    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        SafeWeakIncrement();
    }
    WeakPtr(WeakPtr&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeWeakIncrement();
        other.Reset();
    }

    WeakPtr(const SharedPtr<T>& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeWeakIncrement();
    };

    WeakPtr& operator=(const WeakPtr& other) {
        SafeWeakDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeWeakIncrement();
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        SafeWeakDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeWeakIncrement();
        other.Reset();
        return *this;
    };

    WeakPtr& operator=(SharedPtr<T>& other) {
        SafeWeakDecrement();
        block_ = other.block_;
        ptr_ = other.ptr_;
        SafeWeakIncrement();
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(SharedPtr<Y>& other) {
        SafeWeakDecrement();
        block_ = other.block_;
        ptr_ = other.ptr_;
        SafeWeakIncrement();
        return *this;
    }

    ~WeakPtr() {
        SafeWeakDecrement();
    }

    void PrettyReset() {
        ptr_ = nullptr;
        block_ = nullptr;
    }
    void Reset() {
        SafeWeakDecrement();
        ptr_ = nullptr;
        block_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    void SafeWeakIncrement() {
        if (block_ != nullptr) {
            block_->WeakIncrement();
        }
    }
    void SafeWeakDecrement() {
        if (block_ != nullptr) {
            block_->WeakDecrement();
        }
    }

    void SafeWeakLightDecrement() {
        if (block_ != nullptr) {
            block_->WeakLightDecrement();
        }
    }

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetStrongCount();
    }
    bool Expired() const {
        if (ptr_ == nullptr || block_ == nullptr) {
            return true;
        }
        return block_->IsObjExpired();
    }

    SharedPtr<T> Lock() const {
        SharedPtr<T> sp = SharedPtr<T>();
        if (ptr_ == nullptr) {
            return sp;
        }
        sp.block_ = block_;
        sp.ptr_ = ptr_;
        sp.SafeIncrement();
        return sp;
    };

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class EnableSharedFromThis;

    template <typename U>
    friend class WeakPtr;

private:
    T* ptr_;
    BaseBlock* block_;
};
