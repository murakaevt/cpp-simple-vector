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

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> temp(size);
        std::fill(temp.Get(), temp.Get() + size, 0);
        vector_.swap(temp);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        if (size == 0) {
            ArrayPtr<Type> temp(size);
            vector_.swap(temp);
            size_ = size;
            capacity_ = size;
        }
        else {
            ArrayPtr<Type> temp(size);
            std::fill(temp.Get(), temp.Get() + size, value);
            vector_.swap(temp);
            size_ = size;
            capacity_ = size;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        ArrayPtr<Type> temp(init.size());
        std::copy(init.begin(), init.end(), temp.Get());
        vector_.swap(temp);
        size_ = init.size();
        capacity_ = init.size();
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> temp(other.GetSize());
        std::copy(other.begin(), other.end(), temp.begin());
        this->swap(temp);
    }

    SimpleVector(SimpleVector&& other) {
        this->vector_ = std::move(other.vector_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = other.capacity_;

    }

  explicit SimpleVector(ReserveProxyObj obj) {
        ArrayPtr<Type> temp(obj.cap);
        size_ = 0;
        capacity_ = obj.cap;
        vector_.swap(temp);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs) {
            SimpleVector<Type> temp(rhs.GetSize());
            std::copy(rhs.cbegin(), rhs.cend(), temp.begin());
            this->swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (*this != rhs) {
            this->vector_ = std::move(rhs.vector_);//ArrayPtr<Type> temp(rhs.GetSize());
            size_ = rhs.size_; //std::copy(rhs.cbegin(), rhs.cend(), temp.begin());
            capacity_ = rhs.capacity_; //this->swap(temp);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return vector_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;

    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
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
        /*  if (new_size <= capacity_) {
            if (new_size >= size_) {
                std::generate(vector_.Get() + size_, vector_.Get() + new_size, std::move(p));
                size_ = std::move(new_size);
            }
            if (new_size < size_) {
                size_ = std::move(new_size);
            }
        }
        else {
            ArrayPtr<Type> temp(new_size);
            std::generate(temp.Get(), temp.Get() + new_size, std::move(p));
            std::move(std::make_move_iterator(vector_.Get()), std::make_move_iterator(vector_.Get() + size_), temp.Get());
            vector_ = std::move(temp);
            size_ = std::move(new_size);
            capacity_ = size_;
        } */
    
         
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
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

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t shift = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            size_t new_size = size_ + 1;
            std::copy_backward(begin() + std::distance(cbegin(), pos), end(), begin() + new_size);
            size_t shift = std::distance(cbegin(), pos);
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
            temp[std::distance(cbegin(), pos)] = value;
            std::copy(pos, cend(), temp.Get() + std::distance(cbegin(), pos + 1));
            vector_.swap(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }
      
        return Iterator(cbegin() + shift);
    }

    Iterator Insert(Iterator pos, Type&& value) {
        size_t shift = std::distance(begin(), pos);
        if (size_ < capacity_) {
            size_t new_size = size_ + 1;
            std::copy_backward(std::make_move_iterator(begin() + std::distance(begin(), pos)), std::make_move_iterator(end()), begin() + new_size);
            size_t shift = std::distance(begin(), pos);
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
            temp[std::distance(begin(), pos)] = std::move(value);
            std::copy(std::make_move_iterator(pos), std::make_move_iterator(end()), temp.Get() + std::distance(begin(), pos + 1));
            vector_ = std::move(temp);
            size_ = new_size;
            capacity_ = new_capacity;
        }

        return Iterator(cbegin() + shift);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        size_--;
    }

    // Удаляет элемент вектора в указанной позиции
       Iterator Erase(ConstIterator pos) {
        if (size_ == 1) {
            size_--;
        }
        else {
            size_t new_size = size_ - 1;
            auto it = std::next(pos, 1);
            std::copy(it, cend(), begin() + std::distance(cbegin(), pos));
            size_ = new_size;
        }
        return Iterator(cbegin() + std::distance(cbegin(), pos));
    }

       Iterator Erase(Iterator pos) {
           if (size_ == 1) {
               size_--;
           }
           else {
               size_t new_size = size_ - 1;
               auto it = std::next(pos, 1);
               std::copy(std::make_move_iterator(it), std::make_move_iterator(end()), begin() + std::distance(begin(), pos));
               size_ = new_size;
           }
           return Iterator(begin() + std::distance(begin(), pos));
       }

    // Обменивает значение с другим вектором
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

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(vector_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(vector_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(vector_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(vector_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(vector_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
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