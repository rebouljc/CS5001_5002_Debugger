# Task Lists
These lists are not complete by any means. Some tasks may become unnecessary, and new tasks will be added at any time. The assignees are not in stone, anyone can work on anything.  

* Jan 20, 2021 Update. Since the beginning of the project, sinificant progress was made in an Dear ImGUI project. So Dear ImGUI will be the UI framework used for the project. So many of the **Starting GUI** tasks will be obsolete, and some new tasks will be added. 

### GUI Features
Here are some things we might want in the final UI using the Dear ImGUI framework.
- [ ] Implement a text viewer - *James*
- [ ] Develop a set breakpoint system - *Deion*
- [ ] Develop a breakpoint viewer system - *James*
- [ ] Develop a visual indication of the program hitting a breakpoint - *James*
- [ ] Implement a memory viewer/editor - *Deion*
    - [ ] allow the user to view/edit the values in different bases - *Deion*
- [ ] Implement a CPU register viewer/editor - *Deion*
    - [ ] allow the user to view/edit the values in different bases - *Deion*
- [ ] Develop an option to view assembly alongside source code - *Deion*
- [ ] Develop a syntax highlighting system (RegEx?) - *Wayne*
    - [ ] Implement for x86 Assembly  - *Wayne*
    - [ ] Implement for C  - *Wayne*
    - [ ] Implement for C++ - *James*
    - [ ] Implement for other languages (add to this list) ... - *Wayne*
- [ ] Implement a system to select an .exe to debug - *Deion*
- [ ] Implement a system to select a running process to debug  - *Deion*
- [ ] Develop interactive buttons for debugger interactions - *Deion*
    - [ ] Continue  - *Deion*
    - [ ] Step-forward - *Deion*
    - [ ] Stop - *Deion*
- [x] Develop a program processes view - *Wayne*
- [ ] Research for developing an API to do platform specific windows
- [ ] Develop a way to save sessions to disk. (Minimally. We want to avoid a massive project file problem)
- [ ] Implement UTF-8 support

### Starting Debugger and Debugger Functionality
Get started in creating the debugger. The earlier we start the better we will understand how everything should be designed. - *Wayne*

- [ ] Implement a command line interface to interact with the debugger - *Wayne*
- [ ] Implement some kind of communication system with a running debugger for remote controlling - *Wayne*
- [ ] Create a generalized platform layer where all operating system interactions have to go through this layer - *Wayne*
    * This will help make the debugger easy to port to new operating systems
- [ ] Implement Debug Data format parsers
    - [ ] DWARF debug data format
    - [ ] PDB debug data format
    - [ ] Stabs debug data format (I think this will be of less priority than the previous two)
- [ ] Implement a disassembler
    * Target x86 and x86-64 architectures first. Add other architectures as we learn more. 

* Windows
    - [x] Create a debugger application that can attach itself to another program. (Just a basic application. no breakpoints or anything) - *Wayne* 
    - [ ] Implement: Read memory from an attached program - *James*
    - [ ] Implement: Write memory from an attached program - *James*
    - [ ] Implement: Read CPU registers from an attached program - *James*
    - [ ] Implement: Write CPU registers from an attached program - *James*
    - [ ] Implement: Set and manage breakpoints - *James*
    - [ ] Implement: Catch and interpret program exceptions - *James*
    - [x] Implement: Start the debugee program from the debugger - *James*
    - [ ] Implement: Read in C++ source for user interaction - *James*

* Linux 
    - [x] Create a debugger application that can attach itself to another program. *Wayne*
    - [ ] Implement: Read memory from an attached program - *Wayne*
    - [ ] Implement: Write memory from an attached program - *Wayne*
    - [ ] Implement: Read CPU registers from an attached program  - *Wayne*
    - [ ] Implement: Write CPU registers from an attached program  - *Wayne*
    - [ ] Implement: Set and manage breakpoints - *Wayne*
    - [ ] Implement: Catch and interpret program exceptions - *Wayne*
    - [x] Implement: Start the debugee program from the debugger - *Wayne*
    - [ ] Implement: Read in C++ source for user interaction - *Wayne*

