// Copyright (c) 2017-2020 Intel Corporation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __MFX_TRACE2_H__
#define __MFX_TRACE2_H__
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfx_interface.h"

#define MFX_TRACE2_CTX mfx::SourceLocation(__LINE__, __FILE__, __FUNCTION__)

namespace mfx
{
class Trace;
};

extern mfx::Trace _mfx_trace;

namespace mfx
{

class SourceLocation
{
private:
    mfxU32 _line;
    const char* _file_name;
    const char* _function_name;
public:
    SourceLocation() = default;
    SourceLocation(uint_least32_t _line, const char* _file_name, const char* _function_name);
    ~SourceLocation() = default;

    constexpr uint_least32_t line() const noexcept
    {
        return _line;
    }

    constexpr const char* file_name() const noexcept
    {
        return _file_name;
    }

    constexpr const char* function_name() const noexcept
    {
        return _function_name;
    }
};

class TraceBackend;

class Trace
{
public:
    enum class NodeType {
        NONE,
        STRING,
        MAPPING,
        VECTOR,
    };

    class Node
    {
    public:
        NodeType type;
        union
        {
            std::string str;
            std::map <std::string, Node> map;
            std::vector <Node> vec;
        };
        Node();
        Node(const std::string &str);
        Node(const std::map <std::string, Node> &map);
        Node(const std::vector <Node> &vec);
        Node &operator=(const Node &n);
        Node(const Node &n);
        ~Node();
    };

    enum class EventType
    {
        NONE,
        BEGIN,
        END,
        ADD_INFO,
        OTHER1,
        OTHER2,
    };

    class Event
    {
    public:
        SourceLocation sl;
        const char *name = nullptr;
        const char *category = nullptr;
        mfxU64 id; // event group ID
        mfxU64 parentIndex; // = 0 if it is start
        mfxU64 timestamp; // event timestamp
        std::string threadId;
        EventType type;
        std::string description;
        std::map <std::string, Node> map;
    };

    std::vector <Event> events;
    static mfxU64 idCounter;
    std::vector <std::unique_ptr <TraceBackend> > backends;

    void pushEvent(const Trace::Event &e);

    enum
    {
        GENERIC = 0,
    } traceLevel;

    static std::string hex(mfxU64 value);
    static std::string hex(mfxU32 value);
    static std::string hex(mfxU16 value);
    static std::string hex(mfxU8 value);

    class Scope
    {
    private:
        mfxU8 level = GENERIC;
        Event e;
        mfxU64 parentIndex;
        void add_info_pair(const char* key, const std::string &value);
        void add_info_pair(const char* key, const mfxVideoParam &value);
        void add_info_pair(const char* key, const mfxExtBuffer &value);
        void add_info_pair(const char* key, const mfxExtCodingOption2 &value);
        void add_info_pair(const char* key, const MFX_GUID &value);
        void add_info_pair(const char* key, mfxU32 value);
        void add_info_pair(const char* key, void* value);
    public:
        Scope(SourceLocation sl, const char* name, mfxU8 level = GENERIC);
        Scope(SourceLocation sl, const char* name, const char *category, mfxU8 level = GENERIC);
        ~Scope();

        template <typename... Args>
        void add_info(SourceLocation sl, const char *key, Args... args)
        {
            add_info_pair(key, args...);
            Event event(e);
            event.sl = sl;
            event.type = mfx::Trace::EventType::ADD_INFO;
            event.timestamp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
            event.description = key;
            _mfx_trace.pushEvent(event);
        }
    };

public:
    Trace();
    ~Trace();
};

class TraceBackend
{
public:
    virtual void handleEvent(const Trace::Event &e) = 0;
    virtual ~TraceBackend() {}
};

} // namespace mfx

#endif // #ifndef __MFX_TRACE2_H__
