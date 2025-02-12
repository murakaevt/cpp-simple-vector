#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iterator>

struct ReserveProxyObj {
    ReserveProxyObj(size_t new_capacity)
        : cap(new_capacity) {}
    size_t cap;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // ������ ������ �� size ���������, ������������������ ��������� �� ���������
    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size)
    {
        ArrayPtr<Type> temp(size);
        std::fill(temp.Get(), temp.Get() + size, Type{});
        vector_.swap(temp);
    }

    // ������ ������ �� size ���������, ������������������ ��������� value
    SimpleVector(size_t size, const Type& value)
        : size_(size), capacity_(size), vector_(size)
    {
        ArrayPtr<Type> temp(size);
        std::fill(temp.Get(), temp.Get() + size, value);
        vector_.swap(temp);
    }

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), vector_(init.size())
    {
        ArrayPtr<Type> temp(init.size());
        std::copy(init.begin(), init.end(), temp.Get());
        vector_.swap(temp);
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> temp(other.GetSize());
        std::copy(other.begin(), other.end(), temp.begin());
        this->swap(temp);
    }

    SimpleVector(SimpleVector&& other)
        : size_(std::exchange(other.size_, 0)), capacity_(other.capacity_)
    {
        this->vector_ = std::move(other.vector_);
    }

    explicit SimpleVector(ReserveProxyObj obj)
        : size_(0), capacity_(obj.cap), vector_(obj.cap)
    {
        ArrayPtr<Type> temp(obj.cap);
        vector_.swap(temp);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs) {
            SimpleVector<Type> temp(rhs);
            this->swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (*this != rhs) {
            this->vector_ = std::move(rhs.vector_);
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
        }
        return *this;
    }

    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept {
        return size_;
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // ���������� ������ �� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return vector_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return vector_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return vector_[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return vector_[index];
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept {
        size_ = 0;

    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� �������� �� ��������� ��� ���� Type
    void Resize(size_t new_size) noexcept {
        auto p = []() { return Type{}; };
        if (new_size <= size_) {
            size_ = std::move(new_size);
            return;
        }
        else if (capacity_ < new_size) {
            capacity_ = new_size * 2;
            ArrayPtr<Type> temp(capacity_);
            std::move(std::make_move_iterator(vector_.Get()), std::make_move_iterator(vector_.Get() + size_), temp.Get());
            vector_ = std::move(temp);
            size_ = std::move(new_size);
        }
        else {
            std::generate(vector_.Get() + size_, vector_.Get() + new_size, std::move(p));
            size_ = std::move(new_size);
        }
    }

    // ��������� ������� � ����� �������
    // ��� �������� ����� ����������� ����� ����������� �������
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            vector_[size_] = item;
            size_++;
        }
        else {
            size_t new_size = size_ + 1;
            size_t new_capacity = 0;
            if (capacity_ == 0) {
                new_capacity = 1;
            }
            else {
                new_capacity = capacity_ * 2;
            }
            ArrayPtr<Type> temp(new_capacity);
            std::copy(begin(), end(), temp.Get());
            temp[new_size - 1] = item;
            vector_.swap(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            vector_[size_] = std::move(item);
            size_++;
        }
        else {
            size_t new_size = size_ + 1;
            size_t new_capacity = 0;
            if (capacity_ == 0) {
                new_capacity = 1;
            }
            else {
                new_capacity = capacity_ * 2;
            }
            ArrayPtr<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.Get());
            temp[new_size - 1] = std::move(item);
            vector_ = std::move(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // ��������� �������� value � ������� pos.
    // ���������� �������� �� ����������� ��������
    // ���� ����� �������� �������� ������ ��� �������� ���������,
    // ����������� ������� ������ ����������� �����, � ��� ������� ������������ 0 ����� ������ 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t shift = static_cast<size_t>(std::distance(cbegin(), pos));
        if (size_ < capacity_) {
            size_t new_size = size_ + 1;
            std::copy_backward(begin() + shift, end(), begin() + new_size);
            vector_[shift] = value;
            size_ = new_size;
        }
        else {
            size_t new_size = size_ + 1;
            size_t new_capacity = 0;
            if (capacity_ == 0) {
                new_capacity = 1;
            }
            else {
                new_capacity = capacity_ * 2;
            }
            ArrayPtr<Type> temp(new_capacity);
            std::copy(cbegin(), pos, temp.Get());
            temp[shift] = value;
            std::copy(pos, cend(), temp.Get() + static_cast<size_t>(std::distance(cbegin(), pos + 1)));
            vector_.swap(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }

        return Iterator(cbegin() + shift);
    }

    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t shift = static_cast<size_t>(std::distance(begin(), pos));
        if (size_ < capacity_) {
            size_t new_size = size_ + 1;
            std::copy_backward(std::make_move_iterator(begin() + shift), std::make_move_iterator(end()), begin() + new_size);
            vector_[shift] = std::move(value);
            size_ = new_size;
        }
        else {
            size_t new_size = size_ + 1;
            size_t new_capacity = 0;
            if (capacity_ == 0) {
                new_capacity = 1;
            }
            else {
                new_capacity = capacity_ * 2;
            }
            ArrayPtr<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(pos), temp.Get());
            temp[shift] = std::move(value);
            std::copy(std::make_move_iterator(pos), std::make_move_iterator(end()), temp.Get() + static_cast<size_t>(std::distance(begin(), pos + 1)));
            vector_ = std::move(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }

        return Iterator(cbegin() + shift);
    }

    // "�������" ��������� ������� �������. ������ �� ������ ���� ������
    void PopBack() noexcept {
        assert(size_ > 0);
        size_--;
    }

    // ������� ������� ������� � ��������� �������
    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos < cend());
        assert(size_ > 0);
        if (size_ == 1) {
            size_--;
        }
        else {
            size_t new_size = size_ - 1;
            auto it = std::next(pos, 1);
            std::copy(it, cend(), begin() + static_cast<size_t>(std::distance(cbegin(), pos)));
            size_ = new_size;
        }
        return Iterator(cbegin() + static_cast<size_t>(std::distance(cbegin(), pos)));
    }

    Iterator Erase(Iterator pos) {
        assert(pos >= begin() && pos < end());
        assert(size_ > 0);
        if (size_ == 1) {
            size_--;
        }
        else {
            size_t new_size = size_ - 1;
            auto it = std::next(pos, 1);
            std::copy(std::make_move_iterator(it), std::make_move_iterator(end()), begin() + static_cast<size_t>(std::distance(begin(), pos)));
            size_ = new_size;
        }
        return Iterator(begin() + static_cast<size_t>(std::distance(begin(), pos)));
    }

    // ���������� �������� � ������ ��������
    void swap(SimpleVector& other) noexcept {
        vector_.swap(other.vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            capacity_ = new_capacity;
            ArrayPtr<Type> temp(new_capacity);
            std::copy(cbegin(), cend(), temp.Get());
            vector_.swap(temp);
        }
    }

    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator begin() noexcept {
        return Iterator(vector_.Get());
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator end() noexcept {
        return Iterator(vector_.Get() + size_);
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(vector_.Get());
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(vector_.Get() + size_);
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(vector_.Get());
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(vector_.Get() + size_);
    }
private:
    ArrayPtr<Type> vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    return false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() < rhs.GetSize()) {
        return true;
    }
    else if (lhs.GetSize() == rhs.GetSize()) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    return false;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}