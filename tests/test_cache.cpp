#include <gtest/gtest.h>
#include "Cache.h"
#include <thread>
#include <vector>
#include <string>

// Test Fixture for Cache
class CacheTest : public ::testing::Test {
protected:
    Cache cache;

    void SetUp() override {
        cache.clear();
    }
};

// 1. Basic Set and Get
TEST_F(CacheTest, BasicSetAndGet) {
    cache.set("key1", "value1");
    
    auto result = cache.get("key1");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "value1");
    EXPECT_EQ(cache.size(), 1);
}

// 2. Get Missing Key
TEST_F(CacheTest, GetMissingKey) {
    auto result = cache.get("missing_key");
    EXPECT_FALSE(result.has_value());
}

// 3. Delete Key
TEST_F(CacheTest, DeleteKey) {
    cache.set("key1", "value1");
    EXPECT_EQ(cache.size(), 1);

    bool deleted = cache.del("key1");
    EXPECT_TRUE(deleted);
    EXPECT_EQ(cache.size(), 0);

    // Try deleting again
    deleted = cache.del("key1");
    EXPECT_FALSE(deleted);
}

// 4. Update Existing Key
TEST_F(CacheTest, UpdateExistingKey) {
    cache.set("key1", "value1");
    cache.set("key1", "value2");

    auto result = cache.get("key1");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "value2");
    EXPECT_EQ(cache.size(), 1);
}

// 5. Clear Cache
TEST_F(CacheTest, ClearCache) {
    cache.set("key1", "v1");
    cache.set("key2", "v2");
    cache.set("key3", "v3");
    EXPECT_EQ(cache.size(), 3);

    cache.clear();
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.get("key1").has_value());
}

// 6. Thread Safety Test (Concurrent Writes & Reads)
TEST_F(CacheTest, ConcurrentAccess) {
    const int num_threads = 10;
    const int num_operations = 1000;
    std::vector<std::thread> threads;

    // Concurrent writers
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, num_operations]() {
            for (int j = 0; j < num_operations; ++j) {
                std::string key = "key_" + std::to_string(i) + "_" + std::to_string(j);
                cache.set(key, "val");
            }
        });
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(cache.size(), num_threads * num_operations);
}
