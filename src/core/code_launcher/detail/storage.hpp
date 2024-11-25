#pragma once

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace watchman::detail {

// Multithreading FIFO storage
// Each element can be accessed only one time
// It is required to add new element after extracting

template<typename K, typename V>
class ExtractingStorage {
public:
    bool contains(K const & key) const { return storage.contains(key); }

    std::unique_ptr<V> extract(K const & type) {
        std::unique_lock lock(mutex);

        auto & values = storage.at(type);
        storageFree.wait(lock, [&values]() -> bool { return !values.empty(); });

        auto value = std::move(values.back());
        values.pop_back();

        return value;
    }

    void addValue(K const & type, std::unique_ptr<V> value) {
        {
            std::scoped_lock lock(mutex);
            storage.at(type).push_back(std::move(value));
        }
        storageFree.notify_all();
    }

    void addValues(K const & key, std::list<std::unique_ptr<V>> values) {
        storage.emplace(key, std::move(values));
    }

private:
    std::mutex mutex;
    std::condition_variable storageFree;
    std::unordered_map<K, std::list<std::unique_ptr<V>>> storage;
};

}  // namespace watchman::detail
