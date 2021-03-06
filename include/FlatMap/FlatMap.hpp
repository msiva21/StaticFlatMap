#pragma once

#include <cstring>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <utility>


template <
    typename _Key,
    typename _T,
    typename _Compare = std::less<_Key>
>
class FlatMap
    : private _Compare
{
    static_assert(std::is_trivially_copyable<_Key>::value,
            "FlatMap key type must be Trivially Copyable");
    static_assert(std::is_trivially_copyable<_T>::value,
            "FlatMap mapped type must be Trivially Copyable");

    struct Iterator;
    struct PairPtr;
    // struct ConstIterator;

public:
    using key_compare = _Compare;
    using key_type = _Key;
    using mapped_type = _T;
    using value_type = std::pair<const key_type&, mapped_type&>;
    using size_type = std::size_t;
    using iterator = Iterator;
    // using const_iterator = ConstIterator;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = 
    // using pointer = value_type*;
    using const_pointer = const value_type*;

    FlatMap(const key_compare& comp = key_compare()) noexcept
        : _Compare{comp} {}

    // FlatMap(std::initializer_list<value_type> values) noexcept;
    // FlatMap(const FlatMap& other) noexcept;
    // FlatMap(FlatMap&& other) noexcept;
    // FlatMap& operator=(const FlatMap& other) noexcept;
    // FlatMap& operator=(FlatMap&& other) noexcept;
    // bool operator==(const FlatMap& other) noexcept;
    // bool operator!=(const FlatMap& other) noexcept;

    iterator begin() noexcept
    {
        return iterator{&_keys[0], &_vals[0]};
    }

    // const_iterator begin() const noexcept;
    // const_iterator cbegin() const noexcept;
    iterator end() noexcept
    {
        return iterator{&_keys[_size], &_vals[_size]};
    }
    // const_iterator end() const noexcept;
    // const_iterator cend() const noexcept;

    constexpr bool empty() const noexcept
    {
        return _size == 0u;
    }

    constexpr size_type size() const noexcept
    {
        return _size;
    }

    constexpr size_type max_size() const noexcept
    {
        return std::numeric_limits<decltype(size)>::max();
    }

    // void clear() noexcept;

    std::pair<iterator, bool> insert(const value_type& x) noexcept
    {
        auto it = lower_bound(x.first);
        if (it == end() || !key_comp()(x.first, it->first))
            return std::make_pair(it, false);
        // TODO: grow
        difference_type pos = it - begin();
        difference_type cnt = _size - pos + 1;
        memmove(&_keys[pos+1], &_keys[pos], sizeof(*_keys)*cnt);
        memmove(&_vals[pos+1], &_vals[pos], sizeof(*_vals)*cnt);
        ++_size;
        return std::make_pair(iterator{&_keys[pos], &_vals[pos]}, true);
    }

    // template <class P,
    //     typename = typename std::is_constructible<value_type, P&&>::value>>
    // std::pair<iterator, bool> insert(P&& value) noexcept;

    // template <class InputIt>
    // void insert(InputIt first, InputIt last) noexcept;

    // template <class... Args>
    // std::pair<iterator, bool> emplace(Args&&... args) noexcept;

    // iterator find(const key_type& key) noexcept;
    // const_iterator find(const key_type& key) const noexcept;

    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // iterator find(const K& key) noexcept;

    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // const_iterator find(const K& key) const noexcept;

    // iterator erase(const_iterator pos) noexcept;
    // iterator erase(iterator pos) noexcept;
    // iterator erase(const_iterator first, const_iterator last) noexcept;
    // size_type erase(const key_type& key) noexcept;

    // size_type count(const key_type& key) const noexcept;
    // bool contains(const key_type& key) const noexcept;

    iterator lower_bound(const key_type& key) noexcept
    {
        // TODO: dispatch off size
        return _lower_bound_linear(key);
    }
    // const_iterator lower_bound(const key_type& key) const noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // iterator lower_bound(const K& k) noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // const_iterator lower_bound(const K& k) const noexcept;

    // std::pair<iterator, iterator> equal_range(const key_type& key) noexcept;
    // std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // std::pair<iterator, iterator> equal_range(const K& key) noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept;

    // iterator upper_bound(const key_type& key) noexcept;
    // const_iterator upper_bound(const key_type& key) const noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // iterator upper_bound(const K& k) noexcept;
    // template <class K,
    //          class C = _Compare, typename = typename C::is_transparent>
    // const_iterator upper_bound(const K& k) const noexcept;

    void swap(FlatMap& other) noexcept(std::is_nothrow_swappable<_Compare>::value)
    {
        std::swap(_keys,     other._keys);
        std::swap(_vals,     other._vals);
        std::swap(_size,     other._size);
        std::swap(_capacity, other._capacity);
        std::swap(static_cast<_Compare&>(*this), static_cast<_Compare&>(other));
    }

    constexpr key_compare key_comp() const noexcept { return *this; }

private:
    iterator _lower_bound_linear(const key_type& key) noexcept
    {
        auto comp = key_compare();
        const key_type* b = &_keys[0];
        const key_type* e = &_keys[_size];
        const key_type* k;
        for (k = b; k != e; ++k) {
            if (!comp(key, *k))
                break;
        }
        return _make_iterator(k);
    }

    iterator _make_iterator(const key_type* k) noexcept
    {
        auto pos = k - &_keys[0];
        return iterator{k, &_vals[pos]};
    }

    // const key_type* _keys = nullptr;
    // const key_type* _keyend = nullptr;
    // mapped_type*    _vals = nullptr;
    // size_type       _capacity = 0;

    // TODO: use u32 for size and capacity?
    key_type*    _keys = nullptr;
    mapped_type* _vals = nullptr;
    size_type    _size = 0;
    size_type    _capacity = 0;
};

template <typename Key, typename T, typename Compare>
struct FlatMap<Key, T, Compare>::PairPtr : std::pair<const Key&, T&> {
    constexpr PairPtr(const Key& k, T& v) noexcept
        : std::pair<const Key&, T&>{k, v} {}

    const std::pair<const Key&, T&>* operator->() const noexcept
    {
        return this;
    }
};

template <typename Key, typename T, typename Compare>
struct FlatMap<Key, T, Compare>::Iterator {
    constexpr Iterator() noexcept = default;
    constexpr Iterator(const key_type* key, mapped_type* val) noexcept
        : _key{key}, _val{val}
    {}

    std::pair<const key_type&, mapped_type&> operator*() noexcept
    {
        return std::make_pair(*_key, *_val);
    }

    std:: operator->() noexcept
    {
        return &operator*();
    }

    iterator& operator++() noexcept
    {
        ++_key; ++_val;
        return *this;
    }

    iterator operator++(int) noexcept
    {
        iterator tmp{*this};
        ++(*this);
        return tmp;
    }

    iterator& operator+=(difference_type n) noexcept
    {
        _key += n;
        _val += n;
        return *this;
    }

    iterator& operator-=(difference_type n) noexcept
    {
        _key -= n;
        _val -= n;
    }

    iterator operator+(difference_type n) noexcept
    {
        *this += n;
        return *this;
    }

    iterator operator-(difference_type n) noexcept
    {
        *this += n;
        return *this;
    }

    difference_type operator-(iterator other) noexcept
    {
        return _key - other._key;
    }

    friend bool operator==(Iterator a, Iterator b) noexcept
    {
        return a._key == b._key;
    }

    friend bool operator!=(Iterator a, Iterator b) noexcept
    {
        return a._key != b._key;
    }

    friend bool operator<(Iterator a, Iterator b) noexcept
    {
        return a._key < b._key;
    }

    friend bool operator>(Iterator a, Iterator b) noexcept
    {
        return a._key > b._key;
    }

    friend bool operator<=(Iterator a, Iterator b) noexcept
    {
        return a._key <= b._key;
    }

    friend bool operator>=(Iterator a, Iterator b) noexcept
    {
        return a._key >= b._key;
    }

private:
    const key_type* _key;
    mapped_type*    _val;
};

namespace std {

template <class Key, class T, class Compare>
void swap(FlatMap<Key, T, Compare>& x, FlatMap<Key, T, Compare>& y) noexcept
{
    x.swap(y);
}

} // ~std
