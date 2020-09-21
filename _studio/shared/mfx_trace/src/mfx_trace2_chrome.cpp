#include <algorithm>
#include <mfx_trace2_chrome.h>
#include <syscall.h>
#include <unistd.h>

#ifdef MFX_TRACE_ENABLE_CHROME

mfx::Chrome::Chrome()
{
}

void write_event(FILE *file, const mfx::Trace::Event &e) {
    fprintf(file, "{");
    fprintf(file, "\"name\": \"%s\", \"ph\": \"%s\", \"ts\": %llu, \"pid\": %s, \"tid\": \"%s\", \"id\": %llu, ",
        e.name, e.type.c_str(), e.timestamp, e.threadId.c_str(), e.category, e.id);
    if (e.type == "s" || e.type == "f")  // Flow events
    {
        fprintf(file, "\"cat\": \"%s\", \"bp\": \"e\", ", e.description.c_str());
    }
    fprintf(file, "\"args\": {");
    mfxU64 index = 0;
    for (const auto &pair : e.map)
    {
        if (pair.second.type == mfx::Trace::NodeType::STRING)
        {
            fprintf(file, "\"%s\": \"%s\"", pair.first.c_str(), pair.second.str.c_str());
        }
        else
        {
            fprintf(file, "\"%s\": \"\"", pair.first.c_str());
        }
        if (index + 1 != e.map.size())
        {
            fprintf(file, ", ");
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
        if (it->type != "E" || std::string(it->category) == "sync")
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
        if (it->type != "E" || std::string(it->category) != "sync")
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
            e.type = "s";
            e.threadId = it->threadId;
            events.push_back(e);
            e.name = "link";
            e.category = "sync";
            e.timestamp = _mfx_trace.events[it->parentIndex].timestamp;
            e.parentIndex = e.id;
            e.type = "f";
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
