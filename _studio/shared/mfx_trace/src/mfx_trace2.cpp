#include <chrono>
#include <sstream>
#include <thread>
#include <mutex>
#include <mfx_trace2.h>
#include <mfx_trace2_textlog.h>
#include <mfx_trace2_chrome.h>

mfx::Trace _mfx_trace;
mfxU64 mfx::Trace::idCounter = 1;

static std::mutex traceMutex;

mfx::Trace::Trace()
{
    events.reserve(64000);
#ifdef MFX_TRACE_ENABLE_TEXTLOG
    backends.push_back(std::unique_ptr<TraceBackend>(new TextLog));
#endif
#ifdef MFX_TRACE_ENABLE_CHROME
    backends.push_back(std::unique_ptr<TraceBackend>(new Chrome));
#endif
}

void mfx::Trace::pushEvent(const mfx::Trace::Event &e)
{
    std::lock_guard<std::mutex> guard(traceMutex);
    events.push_back(e);
    for (const auto &backend : backends)
    {
        backend->handleEvent(e);
    }
}

mfx::Trace::~Trace()
{
}

mfx::Trace::Node::Node()
    : type(mfx::Trace::NodeType::NONE)
{
}

mfx::Trace::Node::Node(const std::string &str)
    : type(mfx::Trace::NodeType::STRING)
    , str(str)
{
}

mfx::Trace::Node::Node(const std::map <std::string, Node> &str)
    : type(mfx::Trace::NodeType::MAPPING)
    , map(map)
{
}

mfx::Trace::Node::Node(const std::vector <Node> &str)
    : type(mfx::Trace::NodeType::VECTOR)
    , vec(vec)
{
}

mfx::Trace::Node::Node(const mfx::Trace::Node &n)
{
    type = n.type;
    switch (type)
    {
    case NodeType::NONE:
        break;
    case NodeType::STRING:
        new (&str) std::string;
        str = n.str;
        break;
    case NodeType::MAPPING:
        new (&map) std::map <std::string, Node>;
        map = n.map;
        break;
    case NodeType::VECTOR:
        new (&vec) std::vector <Node>;
        vec = n.vec;
        break;
    }
}

mfx::Trace::Node &mfx::Trace::Node::operator=(const mfx::Trace::Node &n)
{
    type = n.type;
    switch (type)
    {
    case NodeType::NONE:
        break;
    case NodeType::STRING:
        new (&str) std::string;
        str = n.str;
        break;
    case NodeType::MAPPING:
        new (&map) std::map <std::string, Node>;
        map = n.map;
        break;
    case NodeType::VECTOR:
        new (&vec) std::vector <Node>;
        vec = n.vec;
        break;
    }
    return *this;
}

mfx::Trace::Node::~Node()
{
    switch (type)
    {
    case NodeType::NONE:
        break;
    case NodeType::STRING:
        str.~basic_string();
        break;
    case NodeType::MAPPING:
        map.~map();
        break;
    case NodeType::VECTOR:
        vec.~vector();
        break;
    }
}

mfx::SourceLocation::SourceLocation(mfxU32 _line, const char* _file_name, const char* _function_name)
    : _line(_line)
    , _file_name(_file_name)
    , _function_name(_function_name)
{
}

mfx::Trace::Scope::Scope(SourceLocation sl, const char* name, mfxU8 level)
    : level(level)
{
    e.sl = sl;
    e.name = name;
    e.timestamp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
    e.threadId = static_cast<std::ostringstream const &>(std::ostringstream() << std::this_thread::get_id()).str();
}

mfx::Trace::Scope::Scope(SourceLocation sl, const char* name, const char *category, mfxU8 level)
    : Scope(sl, name, level)
{
    e.name = name;
    e.parentIndex = 0;
    e.id = idCounter++;
    e.category = category;
    e.timestamp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
    e.type = "B";
    parentIndex = _mfx_trace.events.size();
    _mfx_trace.pushEvent(e);
}

mfx::Trace::Scope::~Scope()
{
    e.parentIndex = parentIndex;
    e.type = "E";
    e.timestamp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
    _mfx_trace.pushEvent(e);
}

std::string mfx::Trace::hex(mfxU32 value)
{
    return static_cast<std::ostringstream const &>(std::ostringstream() << std::hex << value).str();
}

void mfx::Trace::Scope::add_info(const char* key, const std::string &value)
{
    e.map.emplace(key, mfx::Trace::Node(value));
}

void mfx::Trace::Scope::add_info(const char* key, mfxU32 value)
{
    e.map.emplace(key, mfx::Trace::Node(std::to_string(value)));
}

void mfx::Trace::Scope::add_info(const char* key, void *value)
{
    std::string val = static_cast<std::ostringstream const &>(std::ostringstream() << value).str();
    e.map.emplace(key, mfx::Trace::Node(val));
}
