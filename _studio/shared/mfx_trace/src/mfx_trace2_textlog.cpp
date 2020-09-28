#include <mfx_trace2_textlog.h>

#ifdef MFX_TRACE_ENABLE_TEXTLOG

mfx::TextLog::TextLog(const char *filename)
{
    file = fopen(filename, "w");
}

void mfx::TextLog::handleEvent(const mfx::Trace::Event &e)
{
    const char *eventType = "";
    if (e.type == "B")
        eventType = "ENTER";
    else if (e.type == "E")
        eventType = "EXIT";
    else if (e.type == "I")
        eventType = "VARIABLE";
    fprintf(file, "%s:%d - %s %llu %s\n", e.sl.file_name(), e.sl.line(), e.sl.function_name(), e.timestamp, eventType);
    if (e.type == "I")
    {
        auto entry = e.map.find(e.description);
        if (entry != e.map.end())
        {
            if (entry->second.type == Trace::NodeType::STRING)
            {
                fprintf(file, "\t\t%s: %s\n", e.description.c_str(), entry->second.str.c_str());
            }
        }
    }
}

mfx::TextLog::~TextLog()
{
    fclose(file);
}

#endif  // MFX_TRACE_ENABLE_TEXTLOG
