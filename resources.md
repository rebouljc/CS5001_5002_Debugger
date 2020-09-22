# Resources
Here are some resources I started to gather for learning how to make a deubgger.
We might want to decide between debugging on a specific operating system first, and then attempt to port our implementation.
It seems like there is better documentation for debugging on Windows, however I think linux needs
a lot more love in the debugger department.

## Debugger side of things

**Source Code for lldb and gdb**
* lldb: <https://lldb.llvm.org/resources/contributing.html>
* gdb: <https://sourceware.org/git/?p=gdb.git>

Here are some apis that have been written for lldb and gdb. These might give us some idea of what a debugger's
architecture might look like.
* Python api for lldb: <https://lldb.llvm.org/python_reference/index.html>
* Remote serial protocol for gdb: <https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html>

**Windows specific resources**:
* Windows documentation: <https://docs.microsoft.com/en-us/windows/win32/debug/creating-a-basic-debugger>
* Microsoft docs for "Debug Engines" (looks like a connection to visual studio): <https://docs.microsoft.com/en-us/visualstudio/extensibility/debugger/creating-a-custom-debug-engine?view=vs-2019>

**Linux specific resources**
* Linux Blog for how to make a debugger: <https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/>

## GUI side of things
We have a few options we could consider, and maybe we should try a couple before sticking to one option.
Some frameworks that I know of:

* [Qt](https://doc.qt.io/) (PyQt or just plain C++ Qt) (Wayne is particularly familiar with PyQt)
* [ImGui](https://github.com/ocornut/imgui) which is a C++ graphics library that I have heard good things about.