#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <optional>

class Cache {
private:
    size_t capacity;
    // Doubly linked list to store keys (most recently used at the front)
    mutable std::list<std::pair<std::string, std::string>> lru_list;
    // Map to store key -> iterator in the list
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> store;
    
    mutable std::shared_mutex rw_mutex;

public:
    explicit Cache(size_t max_capacity = 100000) : capacity(max_capacity) {}

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
    void clear();
    size_t size() const;
    size_t get_capacity() const { return capacity; }
};
