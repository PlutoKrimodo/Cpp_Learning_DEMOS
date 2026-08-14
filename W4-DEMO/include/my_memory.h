#pragma once
#include <atomic>
#include <new>
#include <utility>
#include <functional>
#include <memory>
#include <type_traits>
#include <cstddef>
#include <exception>

// 控制块基类
struct CtrlBlockBase {
    std::atomic<long> strong_cnt, weak_cnt;
    CtrlBlockBase(long s = 1, long w = 0) : strong_cnt(s), weak_cnt(w) {}
    virtual ~CtrlBlockBase() = default;
    virtual void dispose() = 0;
    virtual void destroy() = 0;
};

// 从裸指针构造的控制块
template<typename T, typename Deleter = std::default_delete<T>>
struct HeapCtrlBlock : public CtrlBlockBase {
    T* ptr;
    Deleter deleter;

    HeapCtrlBlock(T* p, Deleter&& d = Deleter{})
        : CtrlBlockBase(1, 0), ptr(p), deleter(std::move(d)) {}

    void dispose() override {
        if (ptr) {
            deleter(ptr);
            ptr = nullptr;
        }
    }
    void destroy() override {
        delete this;
    }
};

// make_shared 专用控制块（T 对象嵌在控制块之后）
template<typename T>
struct MakeSharedCtrlBlock : public CtrlBlockBase {
    T* ptr;

    // 模板构造函数，完美转发参数给 T 的构造函数
    template<typename... Args>
    MakeSharedCtrlBlock(Args&&... args) : CtrlBlockBase(1, 0) {
        void* mem = reinterpret_cast<char*>(this) + sizeof(MakeSharedCtrlBlock);
        ptr = new(mem) T(std::forward<Args>(args)...);
    }

    void dispose() override {
        if (ptr) {
            ptr->~T();
            ptr = nullptr;
        }
    }
    void destroy() override {
        ::operator delete(this);
    }
};

// 前向声明
template<typename T> class MySharedPtr;
template<typename T> class MyWeakPtr;

// -------------------- MyWeakPtr --------------------
template<typename T>
class MyWeakPtr {
private:
    T* ptr;
    CtrlBlockBase* ctrl;

    void release() {
        if (!ctrl) return;
        if (ctrl->weak_cnt.fetch_sub(1) == 1) {
            if (ctrl->strong_cnt.load() == 0) {
                ctrl->destroy();   // 销毁控制块
            }
        }
        ptr = nullptr;
        ctrl = nullptr;
    }

public:
    MyWeakPtr() : ptr(nullptr), ctrl(nullptr) {}

    // 从 shared_ptr 构造
    MyWeakPtr(const MySharedPtr<T>& sp) : ptr(sp.ptr), ctrl(sp.ctrl) {
        if (ctrl) ctrl->weak_cnt.fetch_add(1);
    }

    // 拷贝构造
    MyWeakPtr(const MyWeakPtr& other) : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->weak_cnt.fetch_add(1);
    }

    // 拷贝赋值（注意自赋值安全）
    MyWeakPtr& operator=(const MyWeakPtr& other) {
        if (this != &other) {
            release();                  // 释放当前资源
            ptr = other.ptr;
            ctrl = other.ctrl;
            if (ctrl) ctrl->weak_cnt.fetch_add(1);
        }
        return *this;
    }

    ~MyWeakPtr() {
        release();
    }

    // 提升为 shared_ptr（若对象存活）
    MySharedPtr<T> lock() const {
        if (!ctrl) return MySharedPtr<T>();
        long cur = ctrl->strong_cnt.load();
        while (cur > 0) {
            if (ctrl->strong_cnt.compare_exchange_weak(cur, cur + 1)) {
                // 成功增加强引用，使用内部构造函数（不增加计数）
                return MySharedPtr<T>(ptr, ctrl, true);
            }
        }
        return MySharedPtr<T>();
    }

    bool expired() const {
        return ctrl ? (ctrl->strong_cnt.load() == 0) : true;
    }
};

// -------------------- MySharedPtr --------------------
template<typename T>
class MySharedPtr {
private:
    T* ptr;
    CtrlBlockBase* ctrl;

    // 内部构造函数：不递增引用计数（用于 make_shared 和 lock）
    MySharedPtr(T* p, CtrlBlockBase* c, bool) : ptr(p), ctrl(c) {}

    friend class MyWeakPtr<T>;

public:
    // 默认构造
    MySharedPtr() : ptr(nullptr), ctrl(nullptr) {}

    // 从裸指针构造（会创建 HeapCtrlBlock）
    explicit MySharedPtr(T* raw_ptr)
        : ptr(raw_ptr), ctrl(new HeapCtrlBlock<T>(raw_ptr)) {}

    // make_shared 静态工厂
    template<typename... Args>
    static MySharedPtr make_shared(Args&&... args) {
        void* raw = ::operator new(sizeof(MakeSharedCtrlBlock<T>) + sizeof(T));
        auto* cb = new(raw) MakeSharedCtrlBlock<T>(std::forward<Args>(args)...);
        return MySharedPtr(cb->ptr, cb, true);
    }

    // 拷贝构造（递增强引用）
    MySharedPtr(const MySharedPtr& other)
        : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) {
            ctrl->strong_cnt.fetch_add(1);
        }
    }

    // 移动构造
    MySharedPtr(MySharedPtr&& other) noexcept
        : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    // 拷贝赋值（使用 copy-and-swap）
    MySharedPtr& operator=(MySharedPtr other) noexcept {
        swap(*this, other);
        return *this;
    }

    // swap 友元
    friend void swap(MySharedPtr& a, MySharedPtr& b) noexcept {
        std::swap(a.ptr, b.ptr);
        std::swap(a.ctrl, b.ctrl);
    }

    // 析构
    ~MySharedPtr() {
        if (!ctrl) return;
        if (ctrl->strong_cnt.fetch_sub(1) == 1) {
            ctrl->dispose();
            if (ctrl->weak_cnt.load() == 0) {
                ctrl->destroy();
            }
        }
    }

    // 接口函数
    explicit operator bool() const {
        return ptr != nullptr;
    }

    T* get() const { return ptr; }
    T* operator->() const { return ptr; }
    T& operator*() const { return *ptr; }

    long use_count() const {
        return ctrl ? ctrl->strong_cnt.load() : 0;
    }

    void reset() {
        MySharedPtr empty;
        swap(*this, empty);
    }
};