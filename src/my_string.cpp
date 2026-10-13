// ============================================================================
// 作业 2：String 类的实现文件
//
// 目前本文件是空的：构建时链接阶段会报 "undefined reference to `String::...`"，
// 这是预期现象。请先到 include/my_string.h 中补好私有数据成员，再在这里实现
// 所有声明过的成员函数与运算符。
//
// 如果你想拆成多个 .cpp 文件，请同步修改根目录 CMakeLists.txt 中的
// STRING_SOURCES 列表。
//
// 实现清单（与 include/my_string.h 一一对应）：
//   [ ] String() / String(const char*) / 拷贝构造 / 移动构造 / 析构
//   [ ] 复制赋值 operator=(const String&) / 移动赋值 operator=(String&&)
//   [ ] operator+ / operator[]（含 const 版本）/ at（含 const 版本）
//   [ ] size / capacity
//   [ ] insert / push_back
//   [ ] c_str / operator const char*
//   [ ] swap
//   [ ] friend operator<< / operator>>
//
// 完成后按 docs/build-and-test.md 的步骤构建、运行测试并做 ASan/UBSan 检查。
// ============================================================================

#include "my_string.h"
#include <iostream>
// TODO: 在此实现 include/my_string.h 中声明的所有成员函数与运算符。
#include <stdexcept>
// using namespace std;

namespace {
constexpr std::size_t kMinCapacity = 16;  // 容量契约：默认构造的空串容量 >= 16

std::size_t raw_length(const char* s) noexcept { /* 数到 '\0' 为止 */ 
    if(s==nullptr){
        return 0;
    }
    std::size_t len=0;
    while(s[len]!='\0'){
        len++; 
    }
    return len;
}

void raw_copy(char* dst, const char* src, std::size_t count) noexcept {/* 逐字节正向复制（memcpy 语义） */
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

// 与 memmove 语义相同：允许 src、dst 指向同一缓冲区的重叠区间
void raw_move(char* dst, const char* src, std::size_t count) noexcept {
    /* 若 dst < src：从前往后复制；否则从后往前复制 */
    if(dst==src || count==0){
        return;
    }
    if (dst < src) {
        for (std::size_t i = 0; i < count; ++i) {
            dst[i] = src[i];
        }
    } else {
        for (std::size_t i = count; i > 0; --i) {
            dst[i - 1] = src[i - 1];
        }
        //防止覆盖
    }
}

std::size_t growth_target(std::size_t current_capacity, std::size_t needed) noexcept {
    // 增长策略：max(所需长度, 当前容量 x 2, kMinCapacity)
    // 这样连续 push_back 的摊销复杂度是 O(1)
    std::size_t target = current_capacity * 2;
    if (target < needed) {
        target = needed;
    }
    if (target < kMinCapacity) {
        target = kMinCapacity;
    }
    return target;
}

}  // namespace


String::String()
    : data_(new char[kMinCapacity + 1]), size_(0), capacity_(kMinCapacity) {
    data_[0] = '\0';
}
String::String(const char* str)
    :data_(nullptr),size_(0),capacity_(0){
    const std::size_t len=raw_length(str);
    const std::size_t cap=(len<kMinCapacity)?kMinCapacity:len;
    char* new_data=new char[cap+1];
    if(len>0){
        raw_copy(new_data,str,len);
    }
    new_data[len]='\0';
    data_=new_data;
    size_=len;
    capacity_=cap;
}


String::String(const String& other) 
    : data_(nullptr), size_(0), capacity_(0) {
    // 1. other 可能是"被移动后"的对象（data_ == nullptr）：按空串处理
    // 2. 分配 max(other.size_, kMinCapacity) + 1 字节（先分配，失败时本对象仍可析构）
    // 3. 逐字节复制 other 的内容，并在末尾写 '\0'
    // 4. 最后才写回 data_ / size_ / capacity_
    const std::size_t len = (other.size_>0) ? other.size_ : 0;  
    const std::size_t cap = (len < kMinCapacity) ? kMinCapacity:len;
    char* new_data = new char[cap + 1];
    if (len > 0) {
        raw_copy(new_data, other.data_, len);  
    }
    new_data[len] = '\0';
    data_ = new_data;
    size_ = len;
    capacity_ = cap;
}
// String::String(String&& other) noexcept
//     : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
//     other.data_ = nullptr;
//     other.size_ = 0;
//     other.capacity_ = 0;
// }
String::~String() {
    delete[] data_;   // delete[] nullptr 合法，被移动后的对象也能正常析构
}



std::size_t String::size() const noexcept { return size_; }
std::size_t String::capacity() const noexcept { return capacity_; }

char& String::operator[](std::size_t index) noexcept { return data_[index]; }
const char& String::operator[](std::size_t index) const noexcept { return data_[index]; }

const char* String::c_str() const noexcept {
    return data_ != nullptr ? data_ : "";   // 被移动后的对象也要返回有效字符串
}
String::operator const char*() const noexcept { return c_str(); }

char& String::at(std::size_t index) {                 // 注意：不是 noexcept
    if (index >= size_) {
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}
// const 版本同理，返回 const char&
const char& String::at(std::size_t index) const{
    if(index>=size_){
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}


void String::swap(String& other) noexcept {
    // 交换 data_、size_、capacity_ 三个成员即可；
    // 自交换（swap(*this)）天然安全，不需要特判
    char* tmp_data = data_;
    data_ = other.data_;
    other.data_ = tmp_data;
    std::size_t tmp_size = size_;
    size_ = other.size_;
    other.size_ = tmp_size;
    std::size_t tmp_cap = capacity_;
    capacity_ = other.capacity_;
    other.capacity_ = tmp_cap;
}
String& String::operator=(const String& other) {
    if (this == &other) {          // 自赋值：直接返回
        return *this;
    }
    String tmp(other);             // 先深拷贝（这里抛出的话，*this 完全没变）
    swap(tmp);                     // 接管副本；tmp 析构时释放原来的缓冲区
    return *this;
}

void String::ensure_capacity(std::size_t needed) {
    if (needed <= capacity_) {
        return;                                          // 容量够，什么都不做
    }
    const std::size_t target = growth_target(capacity_, needed);
    char* fresh = new char[target + 1];                  // 1. 先分配
    /* 2. 把旧内容与结尾 '\0' 拷进 fresh */
    if(size_>0){
        raw_copy(fresh, data_, size_ + 1);   // 包含结尾 '\0'
    } 
    
    fresh[size_] = '\0';            // 写结尾 '\0'
    delete[] data_;                                      // 3. 成功后才释放旧的
    data_ = fresh;
    capacity_ = target;
}
void String::push_back(char ch) {
    ensure_capacity(size_ + 1);   // 可能抛出；抛出时对象保持不变
    data_[size_] = ch;
    ++size_;
    data_[size_] = '\0';
}

String String::operator+(const String& other) const {
    const std::size_t size1=(data_!=nullptr)?size_:0;
    const std::size_t size2=(other.data_!=nullptr)?other.size_:0;
    String result;                                   // 空串
    result.ensure_capacity(size1 + size2);            // 一次分配到位
    // 依次把 this 与 other 的内容拷进 result，写结尾 '\0'，设置 result.size_
    if(size1>0){
        raw_copy(result.data_,data_,size1);
    }
    if(size2>0){
        raw_copy(result.data_+size1,other.data_,size2);
    }
    result.data_[size1+size2]='\0';
    result.size_=size1+size2;

    return result;                                   // 返回值优化 + 移动语义，不会多拷贝
}

void String::insert(std::size_t pos, const String& str) {
    if (pos > size_) {
        throw std::out_of_range("String::insert: pos out of range");   // 先检查，后修改
    }
    const std::size_t added = str.size_;
    if (added == 0) {
        return;                                        // 插入空串（包括自插入空串）
    }

    if (size_ + added > capacity_) {
        // 情况 A：容量不足 —— 在新缓冲区里一次性组装，成功后（最后一步）才释放旧缓冲区
        //
        //    旧缓冲区: [0, pos) | [pos, size_)   (末尾还有 '\0')
        //    新缓冲区: [0, pos) | [str]   | [pos, size_) | '\0'
        //
        //    自插入（str 就是 *this）也安全：此时 str.data_ 仍指向旧缓冲区，
        //    只要把 delete[] data_ 放在最后，读取旧数据就没问题。
        const std::size_t target = growth_target(capacity_, size_ + added);
        char* fresh = new char[target + 1];             
        if(pos>0){
            raw_copy(fresh,data_,pos);
        }
        raw_copy(fresh+pos,str.data_,added);
        if(size_>pos){
            raw_copy(fresh+pos+added,data_+pos,size_-pos);
        }
        fresh[size_ + added] = '\0';                     // 写结尾 '\0'
        delete[] data_;                                  // 释放旧缓冲区
        data_ = fresh;
        capacity_ = target; 

    } else {
        // 情况 B：容量足够 —— 原地腾位置
        //
        //    1. 把 [pos, size_] 整体后移 added 字节（务必包含结尾 '\0'）
        //       必须使用"允许重叠"的移动（memmove 语义）：
        //       目标在源右侧，要从后往前复制，否则会覆盖还没搬走的数据。
        //    2. 再把 str 的内容写入 [pos, pos + added)
        //
        //    自插入时：第 1 步的目标起点 pos + added >= added，
        //    不会覆盖源区间 [0, added)，所以第 2 步仍能读到完整数据。
        raw_move(data_ + pos + added, data_ + pos, size_ - pos + 1);  // 包含 '\0'
        raw_copy(data_ + pos, str.data_, added);    

    }
    size_ += added;                                    // 最后更新长度
}
