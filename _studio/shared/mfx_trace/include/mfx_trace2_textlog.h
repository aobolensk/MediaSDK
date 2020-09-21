#ifndef __MFX_TRACE2_TEXTLOG_H__
#define __MFX_TRACE2_TEXTLOG_H__
#include <mfx_trace2.h>

#ifdef MFX_TRACE_ENABLE_TEXTLOG

extern mfx::Trace _mfx_trace;

namespace mfx
{

class TextLog : public TraceBackend
{
private:
    FILE *file;
public:
    TextLog();
    void handleEvent(const Trace::Event &e) override;
    ~TextLog() override;
};

}

#endif  // MFX_TRACE_ENABLE_TEXTLOG

#endif  // __MFX_TRACE2_TEXTLOG_H__
