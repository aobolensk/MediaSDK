# MediaSDK tracing (mfx_trace2)

## Scoped tracing

For tracing any scope you can use `mfx::Trace::Scope` instance. To trace current scope you need to create object of this class. This object automatically creates events that can be handled by tracing backends.

Event types:
- BEGIN (is created in constructor of `mfx::Trace::Scope` object, means beginning of tracing scope)
- END (is created in destructor of `mfx::Trace::Scope` object, means end of tracing scope)
- ADD_INFO (is created when `event` function is called)
- others... (reserved for creating backend specific events)

```cpp
void Function()
{
    mfx::Trace::Scope tr(MFX_TRACE2_CTX, "Function", "func");
    ...
    {
        mfx::Trace::Scope tr2(MFX_TRACE2_CTX, "Function2 call", "func");
        Function2();
    }
    ...
}
```

Every event has map with additional information. Traces support saving information about variable values which can be used by tracing backends. Saving can be done using `event` method from `mfx::Trace::Scope`. `event` function calls internal implementations for different types `event_pair`. They are dispatched at compile time and it is possible to extend supported types list by adding `event_pair` implementations for other types.

Events exposure:
```cpp
void Function(int param, void *ptr)
{
    mfx::Trace::Scope tr(MFX_TRACE2_CTX, "Function", "func");  // <-- Function begin event
    tr.event(MFX_TRACE2_CTX, "parameter", param);           // <-- add_info event
    tr.event(MFX_TRACE2_CTX, "pointer", ptr);               // <-- add_info event
    ...
}                                                              // <-- Function end event
```

=== event flow diagram placeholder ===

## Tracing backends

Available tracing backends:
- Text log

  How to enable:
  ```sh
  mkdir build && cd build
  cmake -DENABLE_TEXTLOG=ON ..
  echo "Output=0x30" > $HOME/.mfx_trace
  echo "TextLog=<path_to_log_file>" >> $HOME/.mfx_trace  # optional, default log path: /tmp/mfx.log
  ```

- Chrome tracing

  How to enable:
  ```sh
  mkdir build && cd build
  cmake -DENABLE_CHROME_TRACE=ON ..
  ```

  Chrome traces are dumped in current directory with filename `chrome_trace_<pid>.json`.
