#pragma once

#include "common/config.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"

#include <algorithm>
#include <condition_variable>
#include <list>
#include <mutex>

namespace watchman::detail {

class CodeLaunchersStorage {
public:
    bool contains(std::string const & name) const { return codeLaunchers.contains(name); }

    std::unique_ptr<BaseCodeLauncher> getCodeLauncher(Config::CodeLauncherType const & type) {
        std::unique_lock lock(mutex);

        auto & specificCodeLaunchers = codeLaunchers.at(type);

        codeLauncherFree.wait(
            lock, [&specificCodeLaunchers]() -> bool { return !specificCodeLaunchers.empty(); });

        auto launcher = std::move(specificCodeLaunchers.back());
        specificCodeLaunchers.pop_back();

        return launcher;
    }

    void removeById(std::string const & id, Config::CodeLauncherType const & type) {
        std::scoped_lock const lock(mutex);

        auto & containers = codeLaunchers.at(type);

        containers.erase(std::remove_if(containers.begin(), containers.end(),
                                        [id](auto const & c) { return c->containerId == id; }),
                         containers.end());
    }

    void addCodeLauncher(Config::CodeLauncherType const & type,
                         std::unique_ptr<BaseCodeLauncher> launcher) {
        {
            std::scoped_lock lock(mutex);
            codeLaunchers.at(type).push_back(std::move(launcher));
        }
        codeLauncherFree.notify_all();
    }

    void addCodeLaunchers(Config::CodeLauncherType const & type,
                          std::list<std::unique_ptr<BaseCodeLauncher>> && launchers) {
        codeLaunchers.emplace(type, std::move(launchers));
    }

private:
    std::mutex mutex;
    std::condition_variable codeLauncherFree;
    std::unordered_map<Config::CodeLauncherType, std::list<std::unique_ptr<BaseCodeLauncher>>>
        codeLaunchers;
};

}  // namespace watchman::detail
