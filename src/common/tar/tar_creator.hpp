#include "detail/tar_to_stream.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace tar {

// Scope based object
// Put string to constructor and get output in destructor

template<typename T>
class Creator;

template<>
class Creator<std::ostringstream> {
public:
    Creator(std::string & result)
        : m_result(result)
        , m_stream(std::ios::binary | std::ios::trunc) {}

    void addFile(FileInfo const & fileInfo) { detail::tar_to_stream(m_stream, fileInfo); }

    ~Creator() {
        detail::tar_to_stream_tail(m_stream);
        m_result = m_stream.str();
    }

private:
    std::string & m_result;
    std::ostringstream m_stream;
};

template<>
class Creator<std::ofstream> {
public:
    Creator(std::string const & tarName)
        : m_stream(tarName, std::ios::binary | std::ios::trunc) {}

    void addFile(FileInfo const & fileInfo) { detail::tar_to_stream(m_stream, fileInfo); }

    ~Creator() { detail::tar_to_stream_tail(m_stream); }

private:
    std::ofstream m_stream;
};

}  // namespace tar
