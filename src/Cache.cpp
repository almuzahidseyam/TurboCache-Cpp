#include "Cache.h"

void Cache::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    
    auto it = store.find(key);
    if (it != store.end()) {
        // Key exists, update value and move to front
        lru_list.erase(it->second);
    } else {
        // New key, check capacity
        if (store.size() >= capacity) {
            // Evict least recently used (back of the list)
            auto last = lru_list.back();
            store.erase(last.first);
            lru_list.pop_back();
        }
    }
    
    lru_list.push_front({key, value});
    store[key] = lru_list.begin();
}

std::optional<std::string> Cache::get(const std::string& key) const {
    // Note: GET modifies the LRU list, so we need an exclusive lock.
    // While a shared_mutex is kept for 'size' and potential other read-only methods,
    // LRU 'get' implies a write to the list ordering.
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    
    auto it = store.find(key);
    if (it != store.end()) {
        // Move to front (most recently used)
        std::string value = it->second->second;
        lru_list.erase(it->second);
        lru_list.push_front({key, value});
        store[key] = lru_list.begin();
        return value;
    }
    return std::nullopt;
}

bool Cache::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    
    auto it = store.find(key);
    if (it != store.end()) {
        lru_list.erase(it->second);
        store.erase(it);
        return true;
    }
    return false;
}

void Cache::clear() {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    store.clear();
    lru_list.clear();
}

size_t Cache::size() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    return store.size();
}
