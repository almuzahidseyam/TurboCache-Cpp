#pragma once
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <optional>

class Cache {
private:
    std::unordered_map<std::string, std::string> store;
    mutable std::shared_mutex rw_mutex;

public:
    Cache() = default;

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
    void clear();
    size_t size() const;
};
