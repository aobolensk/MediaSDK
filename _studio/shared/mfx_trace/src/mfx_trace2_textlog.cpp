#include <mfx_trace2_textlog.h>

#ifdef MFX_TRACE_ENABLE_TEXTLOG

const char *TEXTLOG_FILENAME = "/tmp/mfx.log";

mfx::TextLog::TextLog()
{
    file = fopen(TEXTLOG_FILENAME, "w");
}

void mfx::TextLog::handleEvent(const mfx::Trace::Event &e)
{
    fprintf(file, "%s:%d - %s %llu %s\n", e.sl.file_name(), e.sl.line(), e.sl.function_name(), e.timestamp, e.parentIndex == 0 ? "ENTER" : "EXIT");
    for (const auto &pair : e.map)
    {
        if (pair.second.type == Trace::NodeType::STRING)
        {
            fprintf(file, "\t\t%s: %s\n", pair.first.c_str(), pair.second.str.c_str());
        }
    }
}

mfx::TextLog::~TextLog()
{
    fclose(file);
}

#endif  // MFX_TRACE_ENABLE_TEXTLOG
