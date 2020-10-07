# Table 1: Anticipated Milestones
| Milestone #      | Description | Completion Date |
| :-------------:| :-----------: | :---------------:|
| 1 | Research and get comfortable with various GUI API's and write a basic application in each of them.  Decide how the backbone is going to interract with the GUI. | Week 0 (Prior to Semester) to End of Week 1 at latest
| 2 | Create a basic debugger application that does at least what are considered the fundamentals: It contains a text viewer; it has basic buttons for basic debugger functions and interractions; it can attach to a running process, select an .exe to debug; it can load source code into the text viewer; and it can set and manage breakpoints which will also involve the capability of being able to read and write the attached program's CPU registers and memory.  | End of Week 9 |
|3| Test and iron out many of the bugs that are likely to occur as a result of Milestone 2 and then enhance the basic application with the nice-to-have features such as catching and interpreting program exceptions, the implementation of a command-line interface, and implementation of the generalized platform layer to encapsulate all of the operating system interractions. | End of Week 13 |
|4| Continue to iron out bugs from Milestone 3 and maybe 2 again and implement the additional remote-control features and potentially attempt to implement the debugger in other languages to enhance portability. |End of Week 14|
|5| Test again for additional bugs in milestones 2,3, and 4, assure that many issues are fixed, and if time persists, attempt to add features which were discussed in the initial requirements, but are not on the list. |End of Semester (Week 15 1/2)|

# Table 2: Timeline
| Task# | Description | Associated Milestone # | Start Date | Completion Date | Primary Responsibility
|:------| :--------: | :---------------------:|:-----------:|:---------------:|:----------------------:|
|1 | Create a basic application using Qt - Windows |1 | Week 0 | Week 1 | Deion        |
| 2| Create a basic application using PyQt - Windows |1 | Week 0 | Week 1 | Deion|
| 3| Create a basic application using ImGui - Windows | 1|  Week 0 | Week 1 | Wayne |
| 4| Create a basic application using GTK - Windows | 1| Week 0 | Week 1 | James |
| 5| Create a basic application using Qt - Linux |1 | Week 0 | Week 1 | Deion |
| 6| Create a basic application using PyQt - Linux |1 | Week 0 | Week 1 | Deion |
| 7| Create a basic application using ImGui - Linux |1 | Week 0 | Week 1  | Wayne |
| 8| Create a basic application using GTK - Linux |1 |Week 0| Week 1 | James |
| 9| Create a debugger application that can attach itself to another program (Just a basic application, no breakpoints or anything) | 1| Week 0 | Week 2 |Wayne |
| 10| Implement a text viewer | 2| Week 1| Week 3 | James|
| 11| Develop a set breakpoint system | 2| Week 1 | Week 4| Deion |
| 12| Develop a breakpoint viewer system |2 | Week 1| Week 4 | James |
| 13| Develop a visual indication of the program hitting a breakpoint |2 | Week 2 | Week4 | James |
| 14| Implement a system to select an .exe to debug | 2| Week 2 | Week 4 | Deion|
| 15| Implement a system to select a running process to debug | 2| Week 2 | Week 5 | Deion |
| 16| Develop interactive buttons for debugger interactions |2 | Week 2 | Week 3|  Deion|
| 17| Continue |2 | Week 3 | Week 5 | Deion|
|18 | Step forward |2 | Week 3 | Week 5| Deion|
| 19| Stop |2 | Week 3 | Week 5 | Deion |
| 20| Implement: Read memory from an attached program -Windows | 2| Week 5| Week 7 | James |
| 21| Implement: Write memory from an attached program -Windows | 2| Week 5| Week 7 | James |
| 22| Implement: Read CPU registers from an attached program -Windows |2 | Week 6| Week 8 | James |
| 23| Implement: Write CPU registers from an attached program -Windows |2 | Week 6| Week 8 | James |
| 24| Implement: Set and manage breakpoints. -Windows | 2| Week 6| Week 8 | James |
| 25| Implement: Start the debugee program from the debugger -Windows | 2| Week 7| Week 9 | James |
| 26| Implement: Read in C++ source for user interaction -Windows |2 | Week 8| Week 9 | James |
| 27| Create a debugger application that can attach itself to another program |2 | Week 5| Week 7 | Wayne |
| 28| Implement: Read CPU registers from an attached program. -Linux |2 | Week 6| Week 8 | Wayne |
| 29| Implement: Write CPU registers from an attached program. -Linux |2 | Week 6| Week 8 | Wayne |
| 30| Implement: Set and manage breakpoints. -Linux |2 | Week 6| Week 8 |Wayne |
| 31| Implement: Start the debugee program from the debugger. -Linux |2 | Week 7| Week 9 | Wayne |
| 32| Implement: Read in C++ source for user interaction -Linux |2 | Week 8| Week 9 | Wayne |
| 33| Implement a memory viewer/editor | 3| Week 5| Week 8 |Deion |
| 34| Allow the user to view/edit the values in different bases | 3| Week 8| Week 10 |  Deion|
| 35| Develop an option to view assembly alongside source code. |3 | Week 6 | Week 8 |  Deion|
| 36| Develop a syntax highlighting system (RegEx?) |3 | Week8 | Week 11 | Wayne |
| 37| Implement for x86 Assembly | 3| Week 5 | Week 11 | Wayne |
| 38| Implement for C | 2,3| Week 5 | Week 11 | Wayne |
| 39| Implement for C++ |2,3 | Week 5 | Week 11 | James |
| 40| Implement a command line interface to interact with the debugger |3 | Week 10 | Week 12 | Wayne |
| 41| Create a generalized platform layer where all operating system interactions have to go through this layer.  This will help make the debugger easy to port to new operating systems. |3 | Week 11 | Week 13 | Wayne |
| 42| Implement: Catch and interpret program exceptions. - Windows |3 | Week 5 | Week 13 | James |
| 43| Implement for other languages (add to this list) |4 | Week 12 | Week 13 | Wayne |
| 44| Implement some kind of communication system with a running debugger for remote controlling |4 | Week 12| 13 | Wayne |
| 45| Test all code from Milestones 2, 3, and 4 and add additional features that were mentioned in requirements that would be great to have to enhance competitiveness (i.e. AI features, error interpretation, suggestions for runtime error repair/code changes), but are not necessary for a functioning debugger, may not be able to be completed by the project deadline, and has do be done at own risk without causing regressions in prior functionality. | 5 | Week 11 | Week 15 | Wayne, James, Deion |
# Table 3: Effort Matrix
| Task# | %Effort James | %Effort Wayne | %Effort Deion |
|:-----:| :-----------: | :------------:|:-------------:|
|1 | 25 |25 | 50 |
|2 | 25 |25 | 50  |
|3 | 25 |50 | 25 |
|4 | 50 |25 | 25 |
|5 | 25 |25 | 50  |
|6 | 25 |25 | 50  |
|7 | 25 |50 | 25 |
|8 | 50 |25 | 25 |
|9 | 25 |50 | 25 |
|10 | 50 |25 | 25 |
|11 | 25 |25 | 50  |
|12 | 50 |25 | 25 |
|13 | 50 |25 | 25 |
|14 | 25 |25 | 50  |
|15 | 25 |25 | 50  |
|16 | 25 |25 | 50  |
|17 | 25 |25 | 50  |
|18 | 25 |25 | 50  |
|19 | 25 |25 | 50  |
|20 | 50 |25 | 25 |
|21 | 50 |25 | 25 |
|22 | 50 |25 | 25 |
|23 | 50 |25 | 25 |
|24 | 50 |25 | 25 |
|25 | 50 |25 | 25 |
|26 | 50 |25 | 25 |
|27 | 25 |50 | 25 |
|28 | 25 |50 | 25 |
|29 | 25 |50 | 25 |
|30 | 25 |50 | 25 |
|31 | 25 |50 | 25 |
|32 | 25 |50 | 25 |
|33 | 25 |25 | 50 |
|34 | 25 |25 | 50  |
|35 | 25 |25 | 50  |
|36 | 25 |50 | 25 |
|37 | 25 |50 | 25 |
|38 | 25 |50 | 25 |
|39 | 50 |25 | 25|
|40 | 25 |50 | 25 |
|41 | 25 |50 | 25 |
|42 | 50 |25 | 25 |
|43 | 25 |50 | 25 |
|44 | 25 |50 | 25 |
|45 | 33 |33 | 34 |