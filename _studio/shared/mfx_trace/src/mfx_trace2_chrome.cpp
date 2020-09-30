#include <algorithm>
#include <mfx_trace2_chrome.h>
#include <syscall.h>
#include <unistd.h>

#ifdef MFX_TRACE_ENABLE_CHROME

mfx::Chrome::Chrome()
{
}

static void print_node(const mfx::Trace::Node &node, FILE *f)
{
    if (node.type == mfx::Trace::NodeType::STRING)
    {
        fprintf(f, "\"%s\"", node.str.c_str());
    }
    else if (node.type == mfx::Trace::NodeType::MAPPING)
    {
        fprintf(f, "{");
        mfxU64 index = 0;
        for (const auto &pair : node.map)
        {
            fprintf(f, "\"%s\": ", pair.first.c_str());
            print_node(pair.second, f);
            if (index + 1 != node.map.size())
            {
                fprintf(f, ",");
            }
            ++index;
        }
        fprintf(f, "}");
    }
    else if (node.type == mfx::Trace::NodeType::VECTOR)
    {
        fprintf(f, "[");
        for (size_t i = 0; i < node.vec.size(); ++i)
        {
            print_node(node.vec[i], f);
            if (i + 1 != node.vec.size())
            {
                fprintf(f, ",");
            }
        }
        fprintf(f, "]");
    }
}

void write_event(FILE *file, const mfx::Trace::Event &e) {
    const char *eventType = "";
    switch (e.type)
    {
    case mfx::Trace::EventType::NONE:
    case mfx::Trace::EventType::ADD_INFO:
        return;
    case mfx::Trace::EventType::BEGIN:
        eventType = "B";
        break;
    case mfx::Trace::EventType::END:
        eventType = "E";
        break;
    case mfx::Trace::EventType::OTHER1:
        eventType = "s";
        break;
    case mfx::Trace::EventType::OTHER2:
        eventType = "f";
        break;
    }
    fprintf(file, "{");
    fprintf(file, "\"name\": \"%s\", \"ph\": \"%s\", \"ts\": %llu, \"pid\": %s, \"tid\": \"%s\", \"id\": %llu, ",
        e.name, eventType, e.timestamp, e.threadId.c_str(), e.category, e.id);
    if (e.type == mfx::Trace::EventType::OTHER1 || e.type == mfx::Trace::EventType::OTHER2)  // Flow events
    {
        fprintf(file, "\"cat\": \"%s\", \"bp\": \"e\", ", e.description.c_str());
    }
    fprintf(file, "\"args\": {");
    mfxU64 index = 0;
    for (const auto &pair : e.map)
    {
        fprintf(file, "\"%s\": ", pair.first.c_str());
        print_node(pair.second, file);
        if (index + 1 != e.map.size())
        {
            fprintf(file, ",");
        }
        ++index;
    }
    fprintf(file, "}},\n");
}

void mfx::Chrome::handleEvent(const mfx::Trace::Event &e)
{
}

mfx::Chrome::~Chrome()
{
    auto events = _mfx_trace.events;
    // Add flow events (arrows)
    std::map <std::string, Trace::Event> flows;
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        if (it->type != mfx::Trace::EventType::END || std::string(it->category) == "sync")
            continue;
        auto syncpIterator = std::find_if(
            it->map.begin(), it->map.end(), [](const std::pair <std::string, Trace::Node> &p)
            { return p.first == "syncp" && p.second.type == Trace::NodeType::STRING; });
        std::string syncp = syncpIterator == it->map.end() ? "0" : syncpIterator->second.str;
        if (syncp != "0")
        {
            flows.insert({syncp, *it});
        }
    }
    for (auto it = _mfx_trace.events.begin(); it != _mfx_trace.events.end(); ++it)
    {
        if (it->type != mfx::Trace::EventType::END || std::string(it->category) != "sync")
            continue;
        auto syncpIterator = std::find_if(
            it->map.begin(), it->map.end(), [](const std::pair <std::string, Trace::Node> &p)
            { return p.first == "syncp" && p.second.type == Trace::NodeType::STRING; });
        std::string syncp = syncpIterator == it->map.end() ? "0" : syncpIterator->second.str;
        auto flowBegin = flows.find(syncp);
        if (flowBegin != flows.end())
        {
            Trace::Event e;
            e.name = "link";
            e.category = flowBegin->second.category;
            e.description = (std::string("link: ") + flowBegin->second.category + "->sync").c_str();
            e.timestamp = flowBegin->second.timestamp;
            e.parentIndex = 0;
            e.id = Trace::idCounter++;
            e.type = mfx::Trace::EventType::OTHER1;
            e.threadId = it->threadId;
            events.push_back(e);
            e.name = "link";
            e.category = "sync";
            e.timestamp = _mfx_trace.events[it->parentIndex].timestamp;
            e.parentIndex = e.id;
            e.type = mfx::Trace::EventType::OTHER2;
            events.push_back(e);
            flows.erase(flowBegin);
        }
    }
    char buffer[64] = {};
    snprintf(buffer, sizeof(buffer), "chrome_trace_%lu.json", syscall(SYS_getpid));
    FILE *chrome_trace_file = fopen(buffer, "w");
    fprintf(chrome_trace_file, "[\n");
    for (Trace::Event &e : events)
    {
        write_event(chrome_trace_file, e);
    }
    fclose(chrome_trace_file);
}

#endif  // MFX_TRACE_ENABLE_CHROME
