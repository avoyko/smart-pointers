#pragma once

#include "sw_fwd.h"

#include <cstddef>
#include <iostream>

struct BaseBlock {

    virtual void StrongIncrement() = 0;
    virtual void StrongDecrement() = 0;
    virtual void WeakIncrement() = 0;
    virtual void WeakDecrement() = 0;
    virtual void WeakLightDecrement() = 0;
    virtual bool IsObjExpired() = 0;
    virtual size_t GetStrongCount() = 0;
    virtual size_t GetWeakCount() = 0;
    virtual ~BaseBlock(){};
};

template <typename T>
struct CBlockPtr : BaseBlock {
public:
    CBlockPtr() = default;

    CBlockPtr(T* other) : strong_cnt(1), weak_cnt(0), obj(other), obj_is_expired(false){};

    void StrongIncrement() override {
        ++strong_cnt;
    }

    void StrongDecrement() override {
        --strong_cnt;
        if (strong_cnt == 0) {
            TryDeleteObj();
            TryDeleteThis();
        }
    }

    void WeakIncrement() override {
        ++weak_cnt;
    }

    void WeakDecrement() override {
        --weak_cnt;
        if (weak_cnt == 0 && strong_cnt == 0) {
            delete this;
        }
    }

    bool IsObjExpired() override {
        return obj_is_expired;
    }

    size_t GetStrongCount() override {
        return strong_cnt;
    }

    size_t GetWeakCount() override {
        return strong_cnt;
    }

    void WeakLightDecrement() override {
        --weak_cnt;
    }

    void TryDeleteObj() {
        if (!obj_is_expired) {
            delete obj;
            obj = nullptr;
            obj_is_expired = true;
        }
    }

    void TryDeleteThis() {
        if (weak_cnt == 0) {
            delete this;
        }
    }

    ~CBlockPtr(){};

    size_t strong_cnt;
    size_t weak_cnt;
    bool obj_is_expired;
    T* obj;
};

template <typename T, typename... Args>
struct CBlockObj : BaseBlock {
public:
    CBlockObj(Args&&... args) : strong_cnt(1), weak_cnt(0), obj_is_expired(false) {
        new (&buffer) T(std::forward<Args>(args)...);
    };

    void StrongIncrement() override {
        ++strong_cnt;
    }

    void StrongDecrement() override {
        --strong_cnt;
        if (strong_cnt == 0) {
            TryDeleteObj();
            TryDeleteThis();
        }
    }

    void WeakIncrement() override {
        ++weak_cnt;
    }

    void WeakDecrement() override {
        --weak_cnt;
        if (strong_cnt == 0 && weak_cnt == 0) {
            delete this;
        }
    }

    void WeakLightDecrement() override {
        --weak_cnt;
    }

    bool IsObjExpired() {
        return obj_is_expired;
    }

    size_t GetStrongCount() override {
        return strong_cnt;
    }

    size_t GetWeakCount() override {
        return strong_cnt;
    }

    void TryDeleteObj() {
        obj_is_expired = true;
        reinterpret_cast<T*>(&buffer)->~T();
    }

    void TryDeleteThis() {
        if (weak_cnt == 0) {
            delete this;
        }
    }

    ~CBlockObj(){};

    size_t strong_cnt;
    size_t weak_cnt;
    bool obj_is_expired;
    std::aligned_storage_t<sizeof(T), alignof(T)> buffer;
};

class ESFTBase {};

template <typename T>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<T> SharedFromThis() {
        return weak_this_.Lock();
    }
    SharedPtr<const T> SharedFromThis() const {
        return weak_this_.Lock();
    };

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    };
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(weak_this_);
    };

    ~EnableSharedFromThis() {
        weak_this_.SafeWeakLightDecrement();
        weak_this_.PrettyReset();
    };
    WeakPtr<T> weak_this_;

private:
    template <typename U>
    friend class WeakPtr;
    template <typename U>
    friend class SharedPtr;
};

template <typename T>
class SharedPtr {
public:
    SharedPtr() : ptr_(nullptr), block_(nullptr){};

    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr){};

    explicit SharedPtr(T* ptr) : ptr_(ptr) {
        block_ = new CBlockPtr(ptr_);
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr);
        }
    }

    template <typename U>
    explicit SharedPtr(U* ptr) : ptr_(ptr) {
        block_ = new CBlockPtr(ptr);
        if constexpr (std::is_convertible_v<U*, ESFTBase*>) {
            InitWeakThis(ptr);
        }
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        SafeIncrement();
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        SafeIncrement();
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        other.Reset();
    }

    SharedPtr(SharedPtr&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        other.Reset();
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        SafeIncrement();
    }

    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
    };

    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        SafeDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        return *this;
    }
    SharedPtr& operator=(const SharedPtr& other) {
        SafeDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SafeDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        other.Reset();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SafeDecrement();
        ptr_ = other.ptr_;
        block_ = other.block_;
        SafeIncrement();
        other.Reset();
        return *this;
    }

    ~SharedPtr() {
        SafeDecrement();
    }

    void Reset() {
        SafeDecrement();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    void Reset(T* ptr) {
        SafeDecrement();
        ptr_ = ptr;
        block_ = new CBlockPtr(ptr);
    }

    template <typename U>
    void Reset(U* ptr) {
        SafeDecrement();
        ptr_ = ptr;
        block_ = new CBlockPtr(ptr);
    };

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    };

    void SafeIncrement() {
        if (block_ != nullptr) {
            block_->StrongIncrement();
        }
    }

    void SafeDecrement() {
        if (block_ != nullptr) {
            block_->StrongDecrement();
        }
    }

    T* Get() const {
        if (block_ != nullptr && block_->IsObjExpired()) {
            return nullptr;
        }
        return ptr_;
    }

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetStrongCount();
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    };

private:
    T* ptr_;
    BaseBlock* block_;

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> sp;
    auto block = new CBlockObj<T, Args...>(std::forward<Args>(args)...);
    sp.ptr_ = reinterpret_cast<T*>(&(block->buffer));
    sp.block_ = block;
    if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
        sp.InitWeakThis(sp.ptr_);
    }
    return sp;
}
