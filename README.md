# W1-DEMO

**第一周**，简单运用了c++的基础特性编写了一个以lambda为主的排序命令行程序。

```bash
# -r 降序
./W1-DEMO -r (后续输入数据)

# -l 奇前偶后，同类型升序
。/W1-DEMO -l

# 默认升序
```


# W2-DEMO

**第二周**，运用 C++ 模板、动态内存管理、拷贝控制（拷贝构造、拷贝赋值、析构）、移动语义（移动构造、移动赋值）及迭代器等特性，手写实现了一个类似 `std::vector` 的容器类 `MyVector`。

主要实现了：
- 构造/析构：默认构造、指定大小构造、拷贝构造、移动构造、析构
- 赋值操作：拷贝赋值、移动赋值、`swap`
- 容量相关：`size`、`capacity`、`empty`、`reserve`、`resize`
- 元素访问：`operator[]`、`at`（含边界检查）
- 修改器：`push_back`（左值和右值版本）、`pop_back`、`clear`
- 迭代器：`begin` / `end`（const 和非 const）
- 内部辅助：`destroy_and_free`、`reallocate`（支持移动构造转移元素）

同时附带一个素数筛选的测试程序，用于验证容器的基本功能（存储、遍历等），测试代码位于 `main.cpp` 中。
