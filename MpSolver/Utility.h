////////////////////////////////
/// usage : 1.	utilities.
/// 
/// note  : 1.	
////////////////////////////////

#ifndef AGAIN_SZX_MP_SOLVER_UTILITY_H
#define AGAIN_SZX_MP_SOLVER_UTILITY_H


#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <vector>
#include <random>
#include <functional>

#include <cstring>
#include <ctime>
#include <cmath>

#include "Config.h"


#define UTILITY_NOT_IMPLEMENTED  throw "Not implemented yet!";


// [on] use chrono instead of ctime.
#define UTILITY_TIMER_CPP_STYLE  1


namespace szx {

/// if there is "#define x  y", VERBATIM_STRINGIFY(x) will get "x".
#define VERBATIM_STRINGIFY(x)  #x
/// if there is "#define x  y", RESOLVED_STRINGIFY(x) will get "y".
#define RESOLVED_STRINGIFY(x)  VERBATIM_STRINGIFY(x)

#define VERBATIM_CONCAT(a, b)  a##b
#define VERBATIM_CONCAT2(a, b, c)  a##b##c
#define VERBATIM_CONCAT3(a, b, c, d)  a##b##c##d
#define RESOLVED_CONCAT(a, b)  VERBATIM_CONCAT(a, b)
#define RESOLVED_CONCAT2(a, b, c)  VERBATIM_CONCAT2(a, b, c)
#define RESOLVED_CONCAT3(a, b, c, d)  VERBATIM_CONCAT3(a, b, c, d)


template<typename T, typename IndexType = int>
class Arr {
public:
    /// it is always valid before copy assignment due to no reallocation.
    using Iterator = T*;
    using ConstIterator = T const *;

    enum ResetOption { AllBits0 = 0, AllBits1 = -1 };

    explicit Arr() : arr(nullptr), len(0) {}
    explicit Arr(IndexType length) { allocate(length); }
    explicit Arr(IndexType length, T *data) : arr(data), len(length) {}
    explicit Arr(IndexType length, const T &defaultValue) : Arr(length) {
        std::fill(arr, arr + length, defaultValue);
    }
    explicit Arr(std::initializer_list<T> l) : Arr(static_cast<IndexType>(l.size())) {
        std::copy(l.begin(), l.end(), arr);
    }

    Arr(const Arr &a) : Arr(a.len) {
        if (this != &a) { copyData(a.arr); }
    }
    Arr(Arr &&a) : Arr(a.len, a.arr) { a.arr = nullptr; }

    Arr& operator=(const Arr &a) {
        if (this != &a) {
            if (len != a.len) {
                clear();
                init(a.len);
            }
            copyData(a.arr);
        }
        return *this;
    }
    Arr& operator=(Arr &&a) {
        if (this != &a) {
            delete[] arr;
            arr = a.arr;
            len = a.len;
            a.arr = nullptr;
        }
        return *this;
    }

    ~Arr() { clear(); }

    /// allocate memory if it has not been init before.
    bool init(IndexType length) {
        if (arr == nullptr) { // avoid re-init and memory leak.
            allocate(length);
            return true;
        }
        return false;
    }

    /// remove all items.
    void clear() {
        delete[] arr;
        arr = nullptr;
    }

    /// set all data to val. any value other than 0 or -1 is undefined behavior.
    void reset(ResetOption val = ResetOption::AllBits0) { memset(arr, val, sizeof(T) * len); }

    T& operator[](IndexType i) { return arr[i]; }
    const T& operator[](IndexType i) const { return arr[i]; }

    T& at(IndexType i) { return arr[i]; }
    const T& at(IndexType i) const { return arr[i]; }

    Iterator begin() { return arr; }
    Iterator end() { return (arr + len); }
    ConstIterator begin() const { return arr; }
    ConstIterator end() const { return (arr + len); }
    T& front() { return at(0); }
    T& back() { return at(len - 1); }
    const T& front() const { return at(0); }
    const T& back() const { return at(len - 1); }

    IndexType size() const { return len; }
    bool empty() const { return (len == 0); }

protected:
    /// must not be called except init.
    void allocate(IndexType length) {
        // TODO[szx][2]: length > (1 << 32)?
        arr = new T[static_cast<size_t>(length)];
        len = length;
    }

    void copyData(T *data) {
        // TODO[szx][1]: what if data is shorter than arr?
        // OPTIMIZE[szx][8]: use memcpy() if all callers are POD type.
        std::copy(data, data + len, arr);
    }


    T *arr;
    IndexType len;
};

template<typename T, typename IndexType = int>
class Arr2D {
public:
    /// it is always valid before copy assignment due to no reallocation.
    using Iterator = T*;
    using ConstIterator = T const *;

    enum ResetOption { AllBits0 = 0, AllBits1 = -1 };

    explicit Arr2D() : arr(nullptr), len1(0), len2(0), len(0) {}
    explicit Arr2D(IndexType length1, IndexType length2) { allocate(length1, length2); }
    explicit Arr2D(IndexType length1, IndexType length2, T *data)
        : arr(data), len1(length1), len2(length2), len(length1 * length2) {}
    explicit Arr2D(IndexType length1, IndexType length2, const T &defaultValue) : Arr2D(length1, length2) {
        std::fill(arr, arr + len, defaultValue);
    }

    Arr2D(const Arr2D &a) : Arr2D(a.len1, a.len2) {
        if (this != &a) { copyData(a.arr); }
    }
    Arr2D(Arr2D &&a) : Arr2D(a.len1, a.len2, a.arr) { a.arr = nullptr; }

    Arr2D& operator=(const Arr2D &a) {
        if (this != &a) {
            if (len != a.len) {
                clear();
                init(a.len1, a.len2);
            } else {
                len1 = a.len1;
                len2 = a.len2;
            }
            copyData(a.arr);
        }
        return *this;
    }
    Arr2D& operator=(Arr2D &&a) {
        if (this != &a) {
            delete[] arr;
            arr = a.arr;
            len1 = a.len1;
            len2 = a.len2;
            len = a.len;
            a.arr = nullptr;
        }
        return *this;
    }

    ~Arr2D() { clear(); }

    /// allocate memory if it has not been init before.
    bool init(IndexType length1, IndexType length2) {
        if (arr == nullptr) { // avoid re-init and memory leak.
            allocate(length1, length2);
            return true;
        }
        return false;
    }

    /// remove all items.
    void clear() {
        delete[] arr;
        arr = nullptr;
    }

    /// set all data to val. any value other than 0 or -1 is undefined behavior.
    void reset(ResetOption val = ResetOption::AllBits0) { memset(arr, val, sizeof(T) * len); }

    IndexType getFlatIndex(IndexType i1, IndexType i2) const { return (i1 * len2 + i2); }

    T* operator[](IndexType i1) { return (arr + i1 * len2); }

    T& at(IndexType i) { return arr[i]; }
    const T& at(IndexType i) const { return arr[i]; }
    T& at(IndexType i1, IndexType i2) { return arr[i1 * len2 + i2]; }
    const T& at(IndexType i1, IndexType i2) const { return arr[i1 * len2 + i2]; }

    Iterator begin() { return arr; }
    Iterator begin(IndexType i1) { return arr + (i1 * len2); }
    ConstIterator begin() const { return arr; }
    ConstIterator begin(IndexType i1) const { return arr + (i1 * len2); }

    Iterator end() { return (arr + len); }
    Iterator end(IndexType i1) { return arr + (i1 * len2) + len2; }
    ConstIterator end() const { return (arr + len); }
    ConstIterator end(IndexType i1) const { return arr + (i1 * len2) + len2; }

    T& front() { return at(0); }
    T& front(IndexType i1) { return at(i1, 0); }
    const T& front() const { return at(0); }
    const T& front(IndexType i1) const { return at(i1, 0); }

    T& back() { return at(len - 1); }
    T& back(IndexType i1) { return at(i1, len - 1); }
    const T& back() const { return at(len - 1); }
    const T& back(IndexType i1) const { return at(i1, len - 1); }

    IndexType size1() const { return len1; }
    IndexType size2() const { return len2; }
    IndexType size() const { return len; }
    bool empty() const { return (len == 0); }

protected:
    /// must not be called except init.
    void allocate(IndexType length1, IndexType length2) {
        len1 = length1;
        len2 = length2;
        len = length1 * length2;
        arr = new T[static_cast<size_t>(len)];
    }

    void copyData(T *data) {
        // TODO[szx][1]: what if data is shorter than arr?
        // OPTIMIZE[szx][8]: use memcpy() if all callers are POD type.
        std::copy(data, data + len, arr);
    }


    T *arr;
    IndexType len1;
    IndexType len2;
    IndexType len;
};


class Random {
public:
    using Generator = std::mt19937;

    
    Random(int seed) : rgen(seed) {}
    Random() : rgen(generateSeed()) {}


    static int generateSeed() {
        return static_cast<int>(std::time(nullptr) + std::clock());
    }

    Generator::result_type operator()() { return rgen(); }

    // pick with probability of (numerator / denominator).
    bool isPicked(unsigned numerator, unsigned denominator) {
        return ((rgen() % denominator) < numerator);
    }

    // pick from [min, max).
    int pick(int min, int max) {
        return ((rgen() % (max - min)) + min);
    }
    // pick from [0, max).
    int pick(int max) {
        return (rgen() % max);
    }

protected:
    Generator rgen;
};


class Timer {
public:
    #if UTILITY_TIMER_CPP_STYLE
    using Millisecond = std::chrono::milliseconds;
    using TimePoint = std::chrono::steady_clock::time_point;
    using Clock = std::chrono::steady_clock;
    #else
    using Millisecond = int;
    using TimePoint = int;
    struct Clock {
        static TimePoint now() { return clock(); }
    };
    #endif // UTILITY_TIMER_CPP_STYLE


    static constexpr double MillisecondsPerSecond = 1000;
    static constexpr double ClocksPerSecond = CLOCKS_PER_SEC;
    static constexpr int ClocksPerMillisecond = static_cast<int>(ClocksPerSecond / MillisecondsPerSecond);


    #if UTILITY_TIMER_CPP_STYLE
    Timer(const Millisecond &duration, const TimePoint &st = Clock::now())
        : startTime(st), endTime(startTime + duration) {}
    #else
    Timer(const Millisecond &duration, const TimePoint &st = Clock::now())
        : startTime(st), endTime(startTime + duration * ClocksPerMillisecond) {}
    #endif // UTILITY_TIMER_CPP_STYLE

    static Millisecond durationInMillisecond(const TimePoint &start, const TimePoint &end) {
        #if UTILITY_TIMER_CPP_STYLE
        return std::chrono::duration_cast<Millisecond>(end - start);
        #else
        return (end - start) / ClocksPerMillisecond;
        #endif // UTILITY_TIMER_CPP_STYLE
    }

    static double durationInSecond(const TimePoint &start, const TimePoint &end) {
        #if UTILITY_TIMER_CPP_STYLE
        return std::chrono::duration_cast<Millisecond>(end - start).count() / MillisecondsPerSecond;
        #else
        return (end - start) / ClocksPerSecond;
        #endif // UTILITY_TIMER_CPP_STYLE
    }

    static Millisecond toMillisecond(double second) {
        #if UTILITY_TIMER_CPP_STYLE
        return Millisecond(static_cast<int>(second * MillisecondsPerSecond));
        #else
        return static_cast<Millisecond>(second * MillisecondsPerSecond);
        #endif // UTILITY_TIMER_CPP_STYLE
    }

    // there is no need to free the pointer. the format of the format string is 
    // the same as std::strftime() in http://en.cppreference.com/w/cpp/chrono/c/strftime.
    static const char* getLocalTime(const char *format = "%Y-%m-%d(%a)%H:%M:%S") {
        static constexpr int DateBufSize = 64;
        static char buf[DateBufSize];
        time_t t = time(NULL);
        tm *date = localtime(&t);
        strftime(buf, DateBufSize, format, date);
        return buf;
    }

    bool isTimeOut() const {
        return (Clock::now() > endTime);
    }

    Millisecond restMilliseconds() const {
        return durationInMillisecond(Clock::now(), endTime);
    }

    double restSeconds() const {
        return durationInSecond(Clock::now(), endTime);
    }

    Millisecond elapsedMilliseconds() const {
        return durationInMillisecond(startTime, Clock::now());
    }

    double elapsedSeconds() const {
        return durationInSecond(startTime, Clock::now());
    }

    const TimePoint& getStartTime() const { return startTime; }
    const TimePoint& getEndTime() const { return endTime; }

protected:
    TimePoint startTime;
    TimePoint endTime;
};

}


#endif // AGAIN_SZX_MP_SOLVER_H
