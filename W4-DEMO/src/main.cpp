#include "my_memory.h"
#include <iostream>

// 用于测试的辅助类
struct TestClass {
    int value;
    TestClass(int v = 0) : value(v) {
        std::cout << "TestClass(" << value << ") constructed\n";
    }
    ~TestClass() {
        std::cout << "TestClass(" << value << ") destroyed\n";
    }
    void print() const {
        std::cout << "value = " << value << std::endl;
    }
};

int main() {
    std::cout << "========== 1. 基本构造与解构 ==========\n";
    {
        MySharedPtr<TestClass> sp1(new TestClass(10));   // 原始指针构造
        std::cout << "use_count: " << sp1.use_count() << "\n";
        sp1->print();
    }   // 离开作用域，析构，应输出销毁

    std::cout << "\n========== 2. 拷贝构造与引用计数 ==========\n";
    {
        MySharedPtr<TestClass> sp2(new TestClass(20));
        MySharedPtr<TestClass> sp3(sp2);   // 拷贝构造
        std::cout << "sp2 use_count: " << sp2.use_count() << "\n"; 
        std::cout << "sp3 use_count: " << sp3.use_count() << "\n";
        sp3->value = 30;
        sp2->print();  // 应输出30，证明共享同一对象
    }

    std::cout << "\n========== 3. 移动构造 ==========\n";
    {
        MySharedPtr<TestClass> sp4(new TestClass(40));
        MySharedPtr<TestClass> sp5(std::move(sp4));   // 移动
        std::cout << "sp4 is " << (sp4.get() ? "not empty" : "empty") << "\n";
        std::cout << "sp5 use_count: " << sp5.use_count() << "\n"; 
        sp5->print();
    }

    std::cout << "\n========== 4. 赋值运算符（拷贝+移动） ==========\n";
    {
        MySharedPtr<TestClass> sp6(new TestClass(50));
        MySharedPtr<TestClass> sp7(new TestClass(60));
        std::cout << "before assign, sp6 count: " << sp6.use_count() << ", sp7 count: " << sp7.use_count() << "\n";
        sp6 = sp7;   // 拷贝赋值
        std::cout << "after copy assign, sp6 count: " << sp6.use_count() << ", sp7 count: " << sp7.use_count() << "\n";
        sp6->value = 70;
        sp7->print();

        MySharedPtr<TestClass> sp8;
        sp8 = std::move(sp7);   // 移动赋值
        std::cout << "after move assign, sp7 is " << (sp7.get() ? "not empty" : "empty") << "\n";
        std::cout << "sp8 count: " << sp8.use_count() << "\n";
    }

    std::cout << "\n========== 5. make_shared 测试 ==========\n";
    {
        auto sp9 = MySharedPtr<TestClass>::make_shared(99);
        std::cout << "make_shared use_count: " << sp9.use_count() << "\n";
        sp9->print();
    } 

    std::cout << "\n========== 6. reset 与 swap ==========\n";
    {
        MySharedPtr<TestClass> sp10(new TestClass(100));
        MySharedPtr<TestClass> sp11(new TestClass(110));
        std::cout << "sp10 count: " << sp10.use_count() << ", sp11 count: " << sp11.use_count() << "\n";
        sp10.reset();   // 释放当前资源
        std::cout << "after reset, sp10 count: " << sp10.use_count() << "\n";
        swap(sp10, sp11);   // 交换
        std::cout << "after swap, sp10 value: "; sp10->print();
        std::cout << "sp10 count: " << sp10.use_count() << ", sp11 count: " << sp11.use_count() << "\n";
    }

    std::cout << "\n========== 7. weak_ptr 功能 ==========\n";
    {
        MySharedPtr<TestClass> sp12(new TestClass(200));
        MyWeakPtr<TestClass> wp1(sp12);   // 从 shared_ptr 构造
        std::cout << "wp1.expired() = " << wp1.expired() << " (should be 0)\n";
        auto sp13 = wp1.lock();
        if (sp13) {
            std::cout << "lock succeeded, use_count = " << sp13.use_count() << "\n";
            sp13->print();
        }

        MyWeakPtr<TestClass> wp2;
        wp2 = wp1;   // 拷贝赋值
        std::cout << "wp2.expired() = " << wp2.expired() << "\n";

        sp12.reset();   // 释放原对象
        std::cout << "after reset sp12, wp1.expired() = " << wp1.expired() << " (should be 1)\n";
        auto sp14 = wp1.lock();
        if (!sp14) {
            std::cout << "lock returns empty, as expected\n";
        }
    }

    std::cout << "\n========== 8. 循环引用测试（应能处理） ==========\n";
    struct Node {
        int data;
        MySharedPtr<Node> next;
        MyWeakPtr<Node> weak_next;
        Node(int d) : data(d) { std::cout << "Node(" << d << ") created\n"; }
        ~Node() { std::cout << "Node(" << data << ") destroyed\n"; }
    };
    {
        auto a = MySharedPtr<Node>::make_shared(1);
        auto b = MySharedPtr<Node>::make_shared(2);
        a->next = b;
        b->weak_next = a;
        std::cout << "a use_count: " << a.use_count() << ", b use_count: " << b.use_count() << "\n";
    }

    std::cout << "\n========== 测试结束 ==========\n";
    return 0;
}