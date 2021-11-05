#pragma once

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class HashTable {
public:
    class HashTableIterator {
    public:
        std::pair<K, V>& operator*() {
            return hash_table_reference_.buckets_[bucket_id_];
        }

        operator V&() {
            return hash_table_reference_.buckets_[bucket_id_].second;
        }

        std::pair<K, V>* operator->() const {
            return &(hash_table_reference_.buckets_[bucket_id_]);
        }

        V& operator=(V value) {
            if (!hash_table_reference_.buckets_busy_[bucket_id_]) {
                hash_table_reference_.CheckAllocation();
                hash_table_reference_.size_++;
            }
            hash_table_reference_.buckets_busy_[bucket_id_] = 1;
            return hash_table_reference_.buckets_[bucket_id_].second = value;
        }

        bool operator==(const HashTableIterator& other) const {
            return &hash_table_reference_ == &other.hash_table_reference_ && bucket_id_ == other.bucket_id_;
        }

        bool operator!=(const HashTableIterator& other) const {
            return !((*this) == other);
        }

        bool operator==(V x) const {
            return hash_table_reference_.buckets_[bucket_id_].second == x;
        }

        HashTableIterator(HashTable& _hash_table_reference, size_t _bucket_id)
            : hash_table_reference_(_hash_table_reference), bucket_id_(_bucket_id) {
        }

        HashTableIterator& operator++() {
            ++bucket_id_;
            hash_table_reference_.GoToNext(bucket_id_);
            return *this;
        }

    private:
        HashTable& hash_table_reference_;
        size_t bucket_id_;
    };

    class HashTableConstIterator {
    public:
        std::pair<K, V>& operator*() const {
            return hash_table_reference_.buckets_[bucket_id_];
        }

        operator V() const {
            return hash_table_reference_.buckets_[bucket_id_].second;
        }

        std::pair<K, V>* operator->() const {
            return &(hash_table_reference_.buckets_[bucket_id_]);
        }

        bool operator==(const HashTableConstIterator& other) const {
            return &hash_table_reference_ == &other.hash_table_reference_ && bucket_id_ == other.bucket_id_;
        }

        bool operator!=(const HashTableConstIterator& other) const {
            return !((*this) == other);
        }

        bool operator==(V x) const {
            return hash_table_reference_.buckets_[bucket_id_].second == x;
        }

        HashTableConstIterator(const HashTable& _hash_table_reference, size_t _bucket_id)
            : hash_table_reference_(_hash_table_reference), bucket_id_(_bucket_id) {
        }

        HashTableConstIterator& operator++() {
            ++bucket_id_;
            hash_table_reference_.GoToNext(bucket_id_);
            return *this;
        }

    private:
        const HashTable& hash_table_reference_;
        size_t bucket_id_;
    };

    HashTableIterator operator[](const K& key) {
        CheckAllocation();
        size_t ptr = GetPtr(key);
        if (!buckets_busy_[ptr]) {
            size_++;
            buckets_busy_[ptr] = 1;
            buckets_[ptr].first = key;
        }
        return HashTableIterator(*this, ptr);
    }

    HashTableConstIterator operator[](const K& key) const {
        return HashTableConstIterator(*this, GetPtr(key));
    }

    HashTableIterator find(const K& key) {
        return HashTableIterator(*this, GetPtr(key));
    }

    V at(const K& key) const {
        return HashTableConstIterator(*this, GetPtr(key))->second;
    }

    HashTableIterator begin() {
        size_t bucket_id_ = 0;
        GoToNext(bucket_id_);
        return HashTableIterator(*this, bucket_id_);
    }

    HashTableIterator end() {
        size_t bucket_id_ = capacity_;
        return HashTableIterator(*this, bucket_id_);
    }

    HashTableConstIterator begin() const {
        size_t bucket_id_ = 0;
        GoToNext(bucket_id_);
        return HashTableConstIterator(*this, bucket_id_);
    }

    HashTableConstIterator end() const {
        size_t bucket_id_ = capacity_;
        return HashTableConstIterator(*this, bucket_id_);
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size() == 0;
    }

    HashTable() {
        size_ = 0;
        capacity_ = 1;
        buckets_busy_ = std::shared_ptr<bool[]>(new bool[capacity_]());
        buckets_ = std::shared_ptr<std::pair<K, V>[]>(new std::pair<K, V>[capacity_]());
    }

    HashTable(const HashTable& other) {
        size_ = other.size_;
        capacity_ = other.capacity_;
        buckets_busy_ = std::shared_ptr<bool[]>(new bool[capacity_]());
        buckets_ = std::shared_ptr<std::pair<K, V>[]>(new std::pair<K, V>[capacity_]());
        for (size_t i = 0; i < capacity_; ++i) {
            buckets_[i] = other.buckets_[i];
            buckets_busy_[i] = other.buckets_busy_[i];
        }
    }

    std::pair<HashTableIterator, bool> insert(const std::pair<K, V>& val) {
        CheckAllocation();
        size_t ptr = GetPtr(val.first);
        if (!buckets_busy_[ptr]) {
            buckets_[ptr] = val;
            buckets_busy_[ptr] = 1;
            ++size_;
            return {HashTableIterator(*this, ptr), true};
        } else {
            return {HashTableIterator(*this, ptr), false};
        }
    }

    std::pair<HashTableIterator, bool> emplace(const K& key, const V& val) {
        return insert(make_pair(key, val));
    }

    void clear() {
        size_ = 0;
        capacity_ = INITIAL_SIZE;
        buckets_busy_ = std::shared_ptr<bool[]>(new bool[capacity_]());
        buckets_ = std::shared_ptr<std::pair<K, V>[]>(new std::pair<K, V>[capacity_]());
    }

private:
    const size_t INITIAL_SIZE = 2;

    size_t size_ = 0;
    size_t capacity_ = 0;
    std::shared_ptr<bool[]> buckets_busy_ = nullptr;
    std::shared_ptr<std::pair<K, V>[]> buckets_ = nullptr;

    size_t GetPtr(const K& key) const {
        auto hash = Hash{}(key);
        size_t ptr = hash % capacity_;
        while (!KeyEqual{}(buckets_[ptr].first, key) && buckets_busy_[ptr]) {
            ++ptr;
            ptr %= capacity_;
        }
        return ptr;
    }

    void GoToNext(size_t& bucket_id_) const {
        while (bucket_id_ < capacity_ && !buckets_busy_[bucket_id_]) {
            ++bucket_id_;
        }
    }

    void CheckAllocation() {
        if (2 * (size_ + 1) >= capacity_) {
            Reallocate();
        }
    }

    void Reallocate() {
        std::shared_ptr<std::pair<K, V>[]> values(new std::pair<K, V>[size_]);
        size_t ptr = 0;
        for (size_t i = 0; i < capacity_; ++i) {
            if (buckets_busy_[i]) {
                values[ptr++] = buckets_[i];
            }
        }
        size_ = 0;
        capacity_ *= 2;
        buckets_busy_ = std::shared_ptr<bool[]>(new bool[capacity_]());
        buckets_ = std::shared_ptr<std::pair<K, V>[]>(new std::pair<K, V>[capacity_]());
        for (size_t i = 0; i < ptr; ++i) {
            insert(values[i]);
        }
    }
};
