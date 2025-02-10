#include "Cache.h"

void Cache::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    store[key] = value;
}

std::optional<std::string> Cache::get(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    auto it = store.find(key);
    if (it != store.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Cache::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    return store.erase(key) > 0;
}

void Cache::clear() {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    store.clear();
}

size_t Cache::size() const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    return store.size();
}
