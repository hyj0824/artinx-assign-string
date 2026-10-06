#include "my_string.h"

#include <limits>
#include <stdexcept>

String::String() : data_(new char[17]), size_(0), capacity_(16) {
    data_[0] = '\0';
}

String::String(const char* str) : data_(nullptr), size_(0), capacity_(0) {
    if (str != nullptr) {
        while (str[size_] != '\0') {
            ++size_;
        }
    }

    if (size_ < 16) {
        capacity_ = 16;
    } else {
        capacity_ = size_;
    }
    data_ = new char[capacity_ + 1];
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = str[i];
    }
    data_[size_] = '\0';
}

// 拷贝构造：新对象申请自己的数组，再复制原对象的字符。
String::String(const String& other)
    : data_(new char[other.capacity_ + 1]),
      size_(other.size_), capacity_(other.capacity_) {
    const char* source = other.c_str();
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = source[i];
    }
    data_[size_] = '\0';
}

// 移动构造：接手原数组，不申请新内存；原对象不再负责释放它。
String::String(String&& other) noexcept
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

String::~String() {
    delete[] data_;
}

// 复制赋值：当前对象已经存在，需要替换它原来的内容。
String& String::operator=(const String& other) {
    if (this == &other) {
        return *this;
    }

    char* fresh = new char[other.capacity_ + 1];
    const char* source = other.c_str();
    for (std::size_t i = 0; i < other.size_; ++i) {
        fresh[i] = source[i];
    }
    fresh[other.size_] = '\0';

    // 新数组准备好后才释放旧数组，分配失败时保留原内容。
    delete[] data_;
    data_ = fresh;
    size_ = other.size_;
    capacity_ = other.capacity_;
    return *this;
}

String& String::operator=(String&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    delete[] data_;
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}

String String::operator+(const String& other) const {
    String result(*this);
    result.insert(result.size_, other);
    return result;
}

char& String::operator[](std::size_t index) noexcept {
    return data_[index];
}

const char& String::operator[](std::size_t index) const noexcept {
    return data_[index];
}

char& String::at(std::size_t index) {
    if (index >= size_) {
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}

const char& String::at(std::size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}

std::size_t String::size() const noexcept {
    return size_;
}

std::size_t String::capacity() const noexcept {
    return capacity_;
}

void String::insert(std::size_t pos, const String& str) {
    if (pos > size_) {
        throw std::out_of_range("String::insert: position out of range");
    }
    if (str.size_ == 0) {
        return;
    }

    constexpr std::size_t max_capacity =
        std::numeric_limits<std::size_t>::max() - 1;
    // 先检查再相加，避免新长度超出整数可表示的范围。
    if (str.size_ > max_capacity - size_) {
        throw std::length_error("String::insert: string too long");
    }
    const std::size_t new_size = size_ + str.size_;

    if (new_size <= capacity_) {
        if (this == &str) {
            // 先保存原字符，避免后移时覆盖自插入所需的内容。
            // 如果拷贝时申请内存失败，当前对象还没有被修改。
            String copy(str);
            insert(pos, copy);
            return;
        }

        // 从后往前搬动尾部，包括结束标记，避免覆盖还没搬动的字符。
        for (std::size_t i = size_ + 1; i > pos; --i) {
            data_[i + str.size_ - 1] = data_[i - 1];
        }
        for (std::size_t i = 0; i < str.size_; ++i) {
            data_[pos + i] = str.data_[i];
        }
        size_ = new_size;
        return;
    }

    std::size_t new_capacity = capacity_;
    if (new_capacity < new_size) {
        new_capacity = new_size;
    }

    // 在新数组中拼好全部内容，再替换旧数组。
    // 即使 str 就是当前对象，复制期间旧数据也仍然完整。
    char* fresh = new char[new_capacity + 1];
    for (std::size_t i = 0; i < pos; ++i) {
        fresh[i] = data_[i];
    }
    for (std::size_t i = 0; i < str.size_; ++i) {
        fresh[pos + i] = str.data_[i];
    }
    for (std::size_t i = pos; i < size_; ++i) {
        fresh[str.size_ + i] = data_[i];
    }
    fresh[new_size] = '\0';

    delete[] data_;
    data_ = fresh;
    size_ = new_size;
    capacity_ = new_capacity;
}

void String::push_back(char ch) {
    if (size_ == capacity_) {
        constexpr std::size_t max_capacity =
            std::numeric_limits<std::size_t>::max() - 1;
        if (capacity_ >= max_capacity) {
            throw std::length_error("String::push_back: string too long");
        }

        std::size_t new_capacity;
        if (capacity_ == 0) {
            new_capacity = 16;
        } else if (capacity_ > max_capacity / 2) {
            new_capacity = max_capacity;
        } else {
            new_capacity = capacity_ * 2;
        }
        char* fresh = new char[new_capacity + 1];
        for (std::size_t i = 0; i < size_; ++i) {
            fresh[i] = data_[i];
        }
        fresh[size_] = '\0';

        // 分配与复制成功后再替换旧缓冲区，失败时原对象不变。
        delete[] data_;
        data_ = fresh;
        capacity_ = new_capacity;
    }

    data_[size_] = ch;
    ++size_;
    data_[size_] = '\0';
}

const char* String::c_str() const noexcept {
    if (data_ == nullptr) {
        return "";
    } else {
        return data_;
    }
}

String::operator const char*() const noexcept {
    return c_str();
}

// 只交换地址、长度和容量，不申请内存，也不复制字符。
void String::swap(String& other) noexcept {
    if (this == &other) {
        return;
    }

    char* saved_data = data_;
    data_ = other.data_;
    other.data_ = saved_data;

    std::size_t saved_size = size_;
    size_ = other.size_;
    other.size_ = saved_size;

    std::size_t saved_capacity = capacity_;
    capacity_ = other.capacity_;
    other.capacity_ = saved_capacity;
}
