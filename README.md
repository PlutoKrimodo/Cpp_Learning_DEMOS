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

# W3-DEMO

**第三周**，运用 C++17 标准库（STL）及现代 C++ 特性，实现了一个命令行文本词频统计工具 `wfreq`。程序能够读取任意纯文本文件（UTF‑8 或 ASCII），自动过滤首尾标点、归一化大小写，统计各单词出现次数，并按频率降序输出 Top 20。

主要特性：

- **智能分词**：基于 `std::find_if_not` 与反向迭代器，仅剔除首尾标点，保留内部连字符（`-`）和撇号（`'`），正确识别 `don't`、`state-of-the-art` 等词。
- **安全编码检测**：以二进制模式读取文件头，自动识别 UTF‑16（LE/BE）及 UTF‑8 with BOM，若不符合要求则给出明确错误提示，避免乱码或统计异常。
- **高效统计**：使用 `std::unordered_map` 进行 O(1) 词频累加，结合 `std::move` 移动语义减少拷贝开销。
- **灵活排序**：利用 `std::sort` 和 Lambda 自定义比较规则，实现降序排列，同频率则按字母升序。
- **友好交互**：支持命令行参数传入文件路径，也支持无参数运行时的交互式输入。

```bash
# 统计指定文件，输出 Top 20 高频词
./wfreq input.txt

# 无参数运行时，程序会提示输入文件路径
./wfreq
```


# W4-DEMO

**第四周**，运用 C++ 内存管理、多态、原子操作、模板及 RAII 等特性，手写实现了一套智能指针组件 `MySharedPtr` 和 `MyWeakPtr`，完整模拟了 `std::shared_ptr` / `std::weak_ptr` 的核心行为。

主要实现了：

- **控制块体系**：设计 `CtrlBlockBase` 抽象基类，派生出 `HeapCtrlBlock`（裸指针构造）和 `MakeSharedCtrlBlock`（对象嵌入控制块，单次内存分配），统一管理强/弱引用计数。
- **引用计数**：使用 `std::atomic<long>` 保证线程安全的强引用计数（`strong_cnt`）和弱引用计数（`weak_cnt`），实现自动资源释放。
- **构造方式**：支持从裸指针构造（采用默认删除器 `std::default_delete`）、拷贝构造、移动构造，以及静态工厂 `make_shared` 完美转发参数，减少内存碎片。
- **赋值与交换**：拷贝赋值使用 copy-and-swap 惯用法，提供 `swap` 友元函数，保证异常安全；移动赋值转移所有权。
- **资源管理**：析构时递减强引用，计数归零则调用 `dispose` 销毁对象；若弱计数也为零则销毁控制块。`reset` 可主动释放资源。
- **弱指针支持**：`MyWeakPtr` 可从 `MySharedPtr` 构造，支持 `expired` 检测和 `lock` 提升为 `MySharedPtr`，使用 CAS 循环安全地增加强引用计数，避免竞态。
- **RAII 与访问**：提供 `get`、`operator->`、`operator*`、`explicit operator bool` 等接口，`use_count` 返回当前强引用数。
- **循环引用处理**：通过 `MyWeakPtr` 打破循环引用（如链表节点中 `weak_next` 指向上一节点），测试用例验证了 `a->next = b; b->weak_next = a;` 后资源能正确释放。

附带的测试程序 `main.cpp` 覆盖了上述所有功能，包括基本构造、拷贝/移动、赋值、`make_shared`、`reset/swap`、弱指针锁定以及循环引用场景，确保智能指针行为与标准库一致。
