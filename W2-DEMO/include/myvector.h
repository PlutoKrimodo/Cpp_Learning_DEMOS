#include<iostream>
#include<cstddef>
#include<utility>
#include<stdexcept>

template <typename T>
class MyVector {
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;


	//默认构造
	MyVector() noexcept = default;

	//带大小的构造
	explicit MyVector(size_type n) {
		if (n == 0) return;

		start_ = static_cast<pointer>(::operator new(n * sizeof(T)));
		finish_ = start_;
		end_of_storage_ = start_ + n;

		for (size_type i = 0; i < n; ++i, ++finish_) {
			new(finish_)T();
		}
	}

	//拷贝构造
	MyVector(const MyVector& other) {
		size_type n = other.size();
		if (n == 0)return;

		start_ = static_cast<pointer>(::operator new(n * sizeof(T)));
		finish_ = start_;
		end_of_storage_ = start_ + n;

		for (pointer p = other.start_; p != other.finish_; ++p,++finish_) {
			new(finish_) T(*p);
		}
	}

	//移动构造
	MyVector(MyVector&& other)noexcept:start_(other.start_),finish_(other.finish_),
	end_of_storage_(other.end_of_storage_){
		other.start_ = other.finish_ = other.end_of_storage_ = nullptr;
	}

	//析构
	~MyVector() {
		destroy_and_free();
	}

	//赋值
	//拷贝赋值
	MyVector& operator=(const MyVector& other) {
		if (this != &other) {
			MyVector temp(other);
			swap(temp);
		}
		return *this;
	}
	//移动赋值
	MyVector& operator=(MyVector&& other)noexcept {
		if (this != &other) {
			destroy_and_free();
			start_ = other.start_;
			finish_ = other.finish_;
			end_of_storage_ = other.end_of_storage_;
			other.start_ = other.finish_ = other.end_of_storage_=nullptr;
		}
		return *this;
	}
	//交换
	void swap(MyVector& other)noexcept {
		using std::swap;
		swap(start_, other.start_);
		swap(finish_, other.finish_);
		swap(end_of_storage_, other.end_of_storage_);
	}

	//size()
	size_type size() const noexcept {
		return static_cast<size_type>(finish_ - start_);
	}
	//capacity()
	size_type capacity() const noexcept {
		return static_cast<size_type>(end_of_storage_ - start_);
	}
	//empty()
	bool empty() const noexcept {
		return (start_ == finish_);
	}
	//reserve
	void reserve(size_type new_cap) {
		if (new_cap > capacity()) {
			reallocate(new_cap);
		}
	}
	//resize
	void resize(size_type new_size) {
		size_type current_size = size();
		if (new_size < current_size) {
			while (finish_ != start_ + new_size) {
				--finish_;
				finish_->~T();
			}
		}
		else if (new_size > current_size) {
			if (new_size > capacity()) {
				reallocate(new_size);
			}
			while (finish_ != start_ + new_size) {
				new(finish_) T();
				++finish_;
			}
		}
	}

	//元素访问
	reference operator[](size_type pos) {
		return start_[pos];
	}
	const_reference operator[](size_type pos)const {
		return start_[pos];
	}
	reference at(size_type pos){
		if (pos >= size()) throw std::out_of_range("MyVector::at");
		return start_[pos];
	}
	const_reference at(size_type pos) const {
		if (pos >= size()) throw std::out_of_range("MyVector::at");
		return start_[pos];
	}

	//关键修改器
	void push_back(const T& value) {
		if (finish_ == end_of_storage_) {
			size_type new_cap = (start_ == nullptr) ?1 : capacity()*2;
			reallocate(new_cap);
		}
		new(finish_) T(value);
		++finish_;
	}

	void push_back(T&& value) {
		if (finish_ == end_of_storage_) {
			size_type new_cap = (start_ == nullptr) ? 1 : capacity()*2;
			reallocate(new_cap);
		}
		new(finish_) T(std::move(value));
		++finish_;
	}

	void pop_back() {
		if (!empty()) {
			--finish_;
			finish_->~T();
		}
	}

	void clear() {
		while (finish_!= start_) {
			--finish_;
			finish_->~T();
		}
	}

	//迭代器
	pointer begin() noexcept{
		return start_;
	}
	const_pointer begin()const noexcept {
		return start_;
	}
	pointer end() noexcept{
		return finish_;
	}
	const_pointer end()const noexcept {
		return finish_;
	}
private:
	pointer start_ = nullptr;
	pointer finish_ = nullptr;
	pointer end_of_storage_ = nullptr;

	
	//析构有效元素
	void destroy_and_free() {
		if (start_) {
			for (pointer p = finish_; p != start_;) {
				--p;
				p->~T();
			}
			::operator delete(start_);
		}
		start_ = finish_ = end_of_storage_ = nullptr;
	}

	//扩容
	void reallocate(size_type new_capacity) {
		if (new_capacity == 0) {
			return;
		}

		pointer new_start = static_cast<pointer>(::operator new(new_capacity * sizeof(T)));
		pointer new_finish = new_start;
		for (pointer p = start_; p != finish_; ++p, ++new_finish) {
			//移动构造
			new(new_finish)T(std::move(*p));
			p->~T();
		}

		::operator delete(start_);
		start_ = new_start;
		finish_ = new_finish;
		end_of_storage_ = new_start + new_capacity;
	}
};

template <typename T>
void swap(MyVector<T>& lhs, MyVector<T>& rhs) noexcept {
	lhs.swap(rhs);
}