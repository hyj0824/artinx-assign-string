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
#include <stdexcept>
#include <utility>

// TODO: 在此实现 include/my_string.h 中声明的所有成员函数与运算符。

String::String() : data_(new char[17]), size_(0), capacity_(16) {
       data_[0] = '\0';
}

String::String(const char* str) : data_(nullptr), size_(0), capacity_(0) {
	if (str == nullptr){
		data_ = new char[1];
		data_[0] = '\0';
	} else {
		std::size_t len = 0;
		while (str[len] != '\0'){
			++len;
		}
		size_ = len;
		capacity_ = len;
		data_ = new char[capacity_ + 1];
		for (std::size_t i = 0;i < len;i++){
			data_[i] = str[i];
		}
		data_[len] = '\0';
	}
}

String::String(const String& other) : data_(nullptr), size_(0), capacity_(0){

	if (other.data_ == nullptr){
		data_ = new char[1];
		data_[0] = '\0';
	} else {
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = new char[capacity_ + 1];
		for (std::size_t i = 0;i <= size_;++i){
			data_[i] = other.data_[i];
		}
	}
}

String::String(String&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
	other.data_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
}

String::~String(){
	delete[] data_;
}

// --operator--

String& String::operator=(const String& other){
	if (this == &other){
		return *this;
	}

	char* new_data = nullptr;
	std::size_t new_size = 0;
	std::size_t new_capacity = 0;

	if (other.data_ == nullptr){
		new_data = new char[1];
		new_data[0] = '\0';
	} else {
		new_size = other.size_;
		new_capacity = other.capacity_;
		new_data = new char[new_capacity + 1];
		for (std::size_t i = 0;i <= new_size;++i){
			new_data[i] = other.data_[i];
		}
	}

	delete[] data_;
	data_ = new_data;
	size_ = new_size;
	capacity_ = new_capacity;

	return *this;
}

String& String::operator=(String&& other) noexcept{

	if (this == &other){
		return *this;
	}

	delete[] data_;
	size_ = other.size_;
	capacity_ = other.capacity_;
	data_ = other.data_;

	other.data_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;

	return *this;

}

String String::operator+(const String& other) const{
	std::size_t new_size = size_ + other.size_;
	String result;

	if (new_size > result.capacity_){
		char* new_data = new char[new_size + 1];
		delete[] result.data_;
		result.data_ = new_data;
		result.capacity_ = new_size;
	}

	std::size_t k = 0;
	for (std::size_t i = 0;i < size_; ++i){
		result.data_[k++] = data_[i];
	}

	for (std::size_t j = 0;j < other.size_; ++j){
		result.data_[k++] = other.data_[j];
	}

	result.data_[k] = '\0';
	result.size_ = new_size;

	return result;
}

char& String::operator[](std::size_t index) noexcept{
	return data_[index];
}

const char& String::operator[](std::size_t index) const noexcept{
	return data_[index];
}

char& String::at(std::size_t index){
	if (index >= size_){
		throw std::out_of_range("String::at: index out of range");
	}
	return data_[index];
}

const char& String::at(std::size_t index) const{
	if (index >= size_){
		throw std::out_of_range("String::at: index out of range");
	}
	return data_[index];
}

std::size_t String::size() const noexcept{
	return size_;
}

std::size_t String::capacity() const noexcept{
	return capacity_;
}

void String::insert(std::size_t pos, const String& str){
	if (pos > size_){
		throw std::out_of_range("String::insert: pos out of range");
	}

	std::size_t new_size = size_ + str.size_;
	std::size_t new_capacity = capacity_ > new_size ? capacity_ : new_size;

	char* new_data = new char[new_capacity + 1];

	std::size_t k = 0;
	for (std::size_t i = 0;i < pos; ++i){
		new_data[k++] = data_[i];
	}
	for (std::size_t i = 0;i < str.size_; ++i){
		new_data[k++] = str.data_[i];
	}
	for (std::size_t i = pos;i < size_; ++i){
		new_data[k++] = data_[i];
	}

	new_data[k] = '\0';

	delete[] data_;
	data_ = new_data;
	size_ = new_size;
	capacity_ = new_capacity;
}

void String::push_back(char ch){
	std::size_t new_size = size_ + 1;
	std::size_t new_capacity = capacity_ > new_size ? capacity_ : new_size;

	char* new_data = new char[new_capacity + 1];

	std::size_t k = 0;
	for (std::size_t i = 0;i < size_; ++i){
		new_data[k++] = data_[i];
	}
	new_data[k++] = ch;
	new_data[k] = '\0';

	delete[] data_;
	data_ = new_data;
	size_ = new_size;
	capacity_ = new_capacity;
}

const char* String::c_str() const noexcept{
	return data_ ? data_ : "";
}

String::operator const char*() const noexcept{
	return c_str();
}

void String::swap(String& other) noexcept{
	if (this == &other){
		return;
	}
	std::swap(data_, other.data_);
	std::swap(size_, other.size_);
	std::swap(capacity_, other.capacity_);
}