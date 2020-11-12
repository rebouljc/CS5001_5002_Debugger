# Resources
Here are some resources I started to gather for learning how to make a deubgger.
We might want to decide between debugging on a specific operating system first, and then attempt to port our implementation.
It seems like there is better documentation for debugging on Windows, however I think linux needs
a lot more love in the debugger department.

## Debugger side of things
**General resources**
* list of debug data formats: <https://en.wikipedia.org/wiki/Debugging_data_format>
* introduction to the DWARF format: <http://www.dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf>


**Source Code for lldb and gdb**
* lldb: <https://lldb.llvm.org/resources/contributing.html>
* gdb: <https://sourceware.org/git/?p=gdb.git>
	* check windows-nat.c:1819 for process attaching code.

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
I believe we should go with C++ Qt as our Graphics framework for three reasons: Qt is cross-platform, Qt is good for more static UI's, C++ means we don't have to package Python with our exe.

To install the Qt tools (free), you will need to download the [Qt Installer](https://www.qt.io/download-qt-installer) and [create a profile](https://login.qt.io/register).

* [Qt Installer](https://www.qt.io/download-qt-installer)
* [Qt Documentation](https://doc.qt.io/)

