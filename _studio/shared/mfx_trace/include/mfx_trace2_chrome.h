#ifndef __MFX_TRACE2_CHROME_H__
#define __MFX_TRACE2_CHROME_H__
#include <mfx_trace2.h>

#ifdef MFX_TRACE_ENABLE_CHROME

extern mfx::Trace _mfx_trace;

namespace mfx
{

class Chrome : public TraceBackend
{
public:
    Chrome();
    void handleEvent(const Trace::Event &e) override;
    ~Chrome() override;
};

}

#endif  // MFX_TRACE_ENABLE_CHROME

#endif  // __MFX_TRACE2_CHROME_H__
