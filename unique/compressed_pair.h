#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.

template <typename T1, typename T2, bool = std::is_empty<T1>::value && !std::is_final_v<T1>,
          bool = std::is_empty<T2>::value && !std::is_final_v<T2>>
class CompressedPair;

template <typename F, typename S>
class CompressedPair<F, S, true, true> : private F, private S {
public:
    CompressedPair() = default;
    CompressedPair(F& first, S& second) : F(first), S(second) {
    }
    CompressedPair(F& first, S&& second) : F(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S& second) : F(std::move(first)), S(second) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    };

    S& GetSecond() {
        return static_cast<const S&>(*this);
    };
};

template <typename F, typename S>
class CompressedPair<F, S, false, false> {
public:
    CompressedPair() : first_(F()), second_(S()){};
    CompressedPair(F& first, S& second) : first_(first), second_(second) {
    }
    CompressedPair(F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    const S& GetSecond() const {
        return second_;
    };

    S& GetSecond() {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false> : private F {
public:
    CompressedPair() : second_(S()){};
    CompressedPair(F& first, S& second) : F(first), second_(second) {
    }
    CompressedPair(F& first, S&& second) : F(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S& second) : F(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    }

    const S& GetSecond() const {
        return second_;
    };

    S& GetSecond() {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true> : private S {
public:
    CompressedPair() : first_(F()){};
    CompressedPair(F& first, S& second) : first_(first), S(second) {
    }
    CompressedPair(F& first, S&& second) : S(std::move(second)), first_(first) {
    }
    CompressedPair(F&& first, S& second) : first_(std::move(first)), S(second) {
    }
    CompressedPair(F&& first, S&& second) : S(std::move(second)), first_(std::move(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    };

    S& GetSecond() {
        return static_cast<S&>(*this);
    };

private:
    F first_;
};

template <typename F>
class CompressedPair<F, F, true, true> {
public:
    CompressedPair() = default;
    CompressedPair(F& first, F& second) : first_(first), second_(second) {
    }
    CompressedPair(F& first, F&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, F& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, F&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    const F& GetSecond() const {
        return second_;
    };

    F& GetSecond() {
        return second_;
    };

private:
    F first_;
    F second_;
};

template <typename F>
class CompressedPair<F, F, false, false> {
public:
    CompressedPair() = default;
    CompressedPair(F& first, F& second) : first_(first), second_(second) {
    }
    CompressedPair(F& first, F&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, F& second) : first_(std::move(first)), second_(second) {
    }
    CompressedPair(F&& first, F&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    const F& GetSecond() const {
        return second_;
    };

    F& GetSecond() {
        return second_;
    };

private:
    F first_;
    F second_;
};
