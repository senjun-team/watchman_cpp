#pragma once

#include <algorithm>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

namespace watchman::detail {

template<typename K, typename V, typename C = std::list<std::unique_ptr<V>>>
class CodeLaunchersStorage {
public:
    bool contains(std::string const & name) const { return codeLaunchers.contains(name); }

    std::unique_ptr<V> getCodeLauncher(K const & type) {
        std::unique_lock lock(mutex);

        auto & specificCodeLaunchers = codeLaunchers.at(type);

        codeLauncherFree.wait(
            lock, [&specificCodeLaunchers]() -> bool { return !specificCodeLaunchers.empty(); });

        auto launcher = std::move(specificCodeLaunchers.back());
        specificCodeLaunchers.pop_back();

        return launcher;
    }

    template<typename U, typename P>
    void removeById(U const & id, K const & type, P && p) {
        std::scoped_lock const lock(mutex);
        auto & containers = codeLaunchers.at(type);
        containers.erase(std::remove_if(containers.begin(), containers.end(), std::forward<P>(p)),
                         containers.end());
    }

    void addCodeLauncher(K const & type, std::unique_ptr<V> launcher) {
        {
            std::scoped_lock lock(mutex);
            codeLaunchers.at(type).push_back(std::move(launcher));
        }
        codeLauncherFree.notify_all();
    }

    void addCodeLaunchers(K const & key, C && launchers) {
        codeLaunchers.emplace(key, std::move(launchers));
    }

private:
    std::mutex mutex;
    std::condition_variable codeLauncherFree;
    std::unordered_map<K, C> codeLaunchers;
};

}  // namespace watchman::detail
