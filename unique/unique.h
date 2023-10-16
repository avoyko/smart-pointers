#pragma once

#include "compressed_pair.h"

#include <cstddef>
#include <memory>

template <typename T>
struct MyCustomDeleter {
    MyCustomDeleter() = default;

    template <typename F>
    MyCustomDeleter(const MyCustomDeleter<F>&){};

    template <typename U>
    void operator()(U* obj) const {
        delete obj;
    }

    ~MyCustomDeleter() = default;
};

template <typename T>
struct MyCustomDeleter<T[]> {
    MyCustomDeleter() = default;

    template <typename F>
    MyCustomDeleter(const MyCustomDeleter<F>&){};

    template <typename U>
    void operator()(U* obj) const {
        delete[] obj;
    }

    ~MyCustomDeleter() = default;
};



template <typename T, typename Deleter = MyCustomDeleter<T>>
class UniquePtr {
public:
    explicit UniquePtr(T* ptr = nullptr) {
        cp_.GetFirst() = ptr;
    };
    UniquePtr(T* ptr, Deleter deleter) : cp_(ptr, std::move(deleter)){};

    template <typename X, typename Y = MyCustomDeleter<X>>
    UniquePtr(UniquePtr<X, Y>&& other) noexcept
        : cp_(other.cp_.GetFirst(), std::move(other.cp_.GetSecond())) {
        other.cp_.GetFirst() = nullptr;
    };

    template <typename X, typename Y = MyCustomDeleter<X>>
    UniquePtr& operator=(UniquePtr<X, Y>&& other) noexcept {
        Reset(other.Release());
        cp_.GetSecond() = std::forward<Deleter>(other.GetDeleter());  //
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    };

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    ~UniquePtr() {
        T* value = cp_.GetFirst();
        cp_.GetSecond()(value);
    };

    T* Release() {
        T* tmp = cp_.GetFirst();
        cp_.GetFirst() = nullptr;
        return tmp;
    };

    void Reset(T* ptr = nullptr) {
        T* to_reset = cp_.GetFirst();
        cp_.GetFirst() = ptr;
        cp_.GetSecond()(to_reset);
    };
    void Swap(UniquePtr& other) {
        std::swap(cp_, other.cp_);
    };

    T* Get() const {
        return cp_.GetFirst();
    };
    Deleter& GetDeleter() {
        return cp_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return cp_.GetSecond();
    };

    explicit operator bool() const {
        return cp_.GetFirst() != nullptr;
    };

    std::add_lvalue_reference_t<T> operator*() const {
        return *cp_.GetFirst();
    };
    T* operator->() const {
        return cp_.GetFirst();
    };

private:
    template <typename X, typename Y>
    friend class UniquePtr;
    CompressedPair<T*, Deleter> cp_;
};


template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) {
        cp_.GetFirst() = ptr;
        Deleter del;
        cp_.GetSecond() = std::move(del);
    };
    UniquePtr(T* ptr, Deleter deleter) : cp_(ptr, std::move(deleter)){};

    template <typename X, typename Y = MyCustomDeleter<X>>
    UniquePtr(UniquePtr<X, Y>&& other) noexcept
        : cp_(other.cp_.GetFirst(), std::move(other.cp_.GetSecond())) {
        other.cp_.GetFirst() = nullptr;
    };

    template <typename X, typename Y = MyCustomDeleter<X>>
    UniquePtr& operator=(UniquePtr<X, Y>&& other) noexcept {
        Reset(other.Release());
        cp_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        this->Reset();
        return *this;
    };

    ~UniquePtr() {
        T* value = cp_.GetFirst();
        cp_.GetSecond()(value);
    };

    T* Release() {
        T* tmp = cp_.GetFirst();
        Reset();
        return tmp;
    };

    void Reset(T* ptr = nullptr) {
        T* to_reset = cp_.GetFirst();
        cp_.GetFirst() = ptr;
        cp_.GetSecond()(to_reset);
    };
    void Swap(UniquePtr& other) {
        std::swap(cp_, other.cp_);
    };

    T* Get() const {
        return cp_.GetFirst();
    };
    Deleter& GetDeleter() {
        return cp_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return cp_.GetSecond();
    };

    explicit operator bool() const {
        return cp_.GetFirst() != nullptr;
    };

    std::add_lvalue_reference_t<T> operator*() const {
        return *cp_.GetFirst();
    };
    T* operator->() const {
        return cp_.GetFirst();
    };

    T& operator[](size_t i) const {
        return *(cp_.GetFirst() + i);
    }

private:
    CompressedPair<T*, Deleter> cp_;
};
