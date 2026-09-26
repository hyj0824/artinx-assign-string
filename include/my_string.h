#ifndef ASSIGNMENT2_MY_STRING_H
#define ASSIGNMENT2_MY_STRING_H

// ============================================================================
// 作业 2：自定义 String 类（基于 char 数组）
//
// 【你需要做什么】
//   1. 在下方 private: 处自行设计数据成员与辅助函数（本文件不提供任何内部实现）；
//   2. 在 src/my_string.cpp 中实现下面声明的所有成员函数与运算符。
//
// 【约定】
//   * 本文件只保留公开接口，函数名、参数、返回类型、const/noexcept 与异常语义
//     均不允许修改，否则随附测试无法通过编译；
//   * capacity() 采用与 std::string 一致的语义：容量不包含结尾的 '\0'，
//     即缓冲区实际大小为 capacity() + 1；
//   * 除“被移动后”的对象外，所有对象都保存以 '\0' 结尾的有效 C 字符串；
//     被移动后的对象处于“有效但内容未指定”的状态，可以析构、可以重新赋值。
//
// 详细要求见 TASKS.md；构建、测试与 ASan/UBSan 用法见 docs/build-and-test.md。
// ============================================================================

#include <cstddef>
#include <iosfwd>

class String {
public:
    // ---- 构造 / 析构（Rule of Five）----
    String();                                     // 空字符串，初始容量至少 16
    String(const char* str);                      // 由 C 字符串构造；nullptr 视为空串
    String(const String& other);                  // 拷贝构造（深拷贝）
    String(String&& other) noexcept;              // 移动构造（窃取缓冲区）
    ~String();

    // ---- 赋值 ----
    String& operator=(const String& other);       // 复制赋值（强异常安全）
    String& operator=(String&& other) noexcept;   // 移动赋值（自移动安全）

    // ---- 拼接 ----
    String operator+(const String& other) const;

    // ---- 下标访问：与 std::string::operator[] 一样不做边界检查 ----
    // index == size() 合法（返回结尾 '\0' 的引用）；index > size() 是未定义行为。
    char& operator[](std::size_t index) noexcept;
    const char& operator[](std::size_t index) const noexcept;

    // ---- 带边界检查的访问：越界抛出 std::out_of_range ----
    char& at(std::size_t index);
    const char& at(std::size_t index) const;

    // ---- 长度 / 容量 ----
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;        // 不含结尾 '\0'

    // ---- 插入 / 追加 ----
    // 在 pos 处插入 str；pos > size() 时抛出 std::out_of_range。
    // 支持 s.insert(pos, s) 自插入；失败时原对象内容不被破坏。
    void insert(std::size_t pos, const String& str);

    // 在末尾追加单个字符，容量不足时自动扩容，追加后仍以 '\0' 结尾。
    void push_back(char ch);

    // ---- 转换为 C 字符串 ----
    const char* c_str() const noexcept;           // 即使对象被移动过也必须返回有效的 C 字符串
    operator const char*() const noexcept;        // 隐式转换

    // ---- 交换全部内容（自交换也必须安全）----
    void swap(String& other) noexcept;

    // ---- 流操作 ----
    friend std::ostream& operator<<(std::ostream& os, const String& str);
    friend std::istream& operator>>(std::istream& is, String& str);

private:
    // TODO: 在这里添加你自己的私有数据成员与辅助函数。
    //       可以自由选择内部表示（例如缓冲区指针 + 长度 + 容量），
    //       只要公开接口的语义满足 TASKS.md 的要求即可。
    char* data_;
    std::size_t size_;
    std::size_t capacity_;
};

#endif  // ASSIGNMENT2_MY_STRING_H
