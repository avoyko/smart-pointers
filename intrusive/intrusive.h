#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    };
    size_t DecRef() {
        --count_;
        return count_;
    };
    size_t RefCount() const {
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    auto operator()(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    void IncRef() {
        counter_.IncRef();
    }

    void DecRef() {
        if (counter_.RefCount() == 0 || counter_.DecRef() == 0) {
            Deleter deleter;
            deleter(static_cast<Derived*>(this));
        }
    }

    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
public:

    IntrusivePtr() : ptr_(nullptr){};
    IntrusivePtr(std::nullptr_t) : ptr_(nullptr){};
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        SafeIncrement(ptr);
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        ptr_ = other.ptr_;
        SafeIncrement();
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        ptr_ = other.ptr_;
        SafeIncrement();
        other.Reset();
    };

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.ptr_;
        SafeIncrement();
    };
    IntrusivePtr(IntrusivePtr&& other) {
        ptr_ = other.ptr_;
        SafeIncrement();
        other.Reset();
    };

    // `operator=`-s

    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this == &other) {
            return *this;
        }
        SafeDecrement();
        ptr_ = other.ptr_;
        SafeIncrement();
        return *this;
    };

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this == &other) {
            return *this;
        }
        SafeDecrement();
        ptr_ = other.ptr_;
        SafeIncrement();
        other.Reset();
        return *this;
    };

    //    template <typename U>
    //    IntrusivePtr& operator=(U other){
    //        if (ptr_ == &other){
    //            return *this;
    //        }
    //        SafeDecrement();
    //        ptr_ = &other;
    //        SafeIncrement();
    //        return *this;
    //    };

    // Destructor
    ~IntrusivePtr() {
        SafeDecrement();
    };

    void SafeDecrement() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
    }

    void SafeIncrement() {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    }

    void SafeDecrement(T* ptr) {
        if (ptr != nullptr) {
            ptr->DecRef();
        }
    }

    void SafeIncrement(T* ptr) {
        if (ptr != nullptr) {
            ptr->IncRef();
        }
    }

    void Reset() {
        SafeDecrement();
        ptr_ = nullptr;
    };
    void Reset(T* ptr) {
        SafeDecrement();
        ptr_ = ptr;
        SafeIncrement();
    };
    void Swap(IntrusivePtr& other) {
        std::swap(*this, other);
    };

    T* Get() const {
        return ptr_;
    };

    void Set(T* ptr) {
        ptr_ = ptr;
    }
    T& operator*() const {
        return *ptr_;
    };
    T* operator->() const {
        return ptr_;
    };
    size_t UseCount() const {
        if (ptr_ == nullptr) {
            return 0;
        }
        return ptr_->RefCount();
    };
    explicit operator bool() const {
        return ptr_ != nullptr;
    };

private:
    template <typename U>
    friend class IntrusivePtr;

    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> ip;
    T* obj = new T(std::forward<Args>(args)...);
    ip.Set(obj);
    ip.SafeIncrement();
    return ip;
};
