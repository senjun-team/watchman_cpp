#include "detail/tar_to_stream.hpp"

#include <sstream>
#include <string>

namespace tar {

// Scope based object
// Put string to constructor and get output in destructor
class Creator {
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

}  // namespace tar
