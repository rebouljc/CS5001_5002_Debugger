
namespace Debugger {
	
	#define DEBUGGER_MAX_PATH 260
	#define DEBUGGER_MAX_PROCESSES 1000

	// Should probably move the debugger init stuff
	// into a function here

	// Not actually sure how I want to go about this archiecture 
	// We might want to have an instance of this stored
	// on the platform layer and call to it when needed??
	// Or should we keep a global instance here?
	// Or should it be stored in Persistant_Vars and the main_ui loop passes the data here?
	// All of these are possible solutions
	// (maybe there are separate sessions, and each session has it's own DebuggerData instance)
	// But we all don't know how debuggers work so this could all be *really* wrong :)
	struct DebuggerData {
		// Should the exe_path memory reside in the DebuggerData struct 
		// or should it be allocated elsewhere?
		char exe_path[DEBUGGER_MAX_PATH]; 
		bool running = false; // we probably want the exit status of a process
		bool debugging = false;
		unsigned long pid = 0;
		unsigned long tid = 0;
	};

	struct Process {
		unsigned long pid;
		char short_name[50];
	};

	int debug_init(DebuggerData* data);

    // So far this API works by checking for debugging events on every loop and then
    // reacting to those events. But some OS API's (like unix ptrace/waitpid) would allow
    // us to wait for the operating system to tell us that there was an event (rather than checking
    // on every loop). Waiting for the OS to tell us (waitpid) is a blocking call, so we would
    // need to create a separate thread to handle the debugger. Which way is more efficient and
    // can we implement them in a OS agnostic manner?
    //
    // A note on threading: If we put the debugger on a separate thread, I don't
    // think we would run into too many race cases. So far it seems that the debugger
    // will write to some memory and the UI reads that memory. Then the UI writes to some other
    // memory and the debugger reads that memory. So they don't write to the same memory 
    // (at least as far as I have gotten with writing this program)
	int debug_loop(DebuggerData* data);
	
	int start_and_debug_exe(DebuggerData* data);
	int get_number_registers();
	unsigned long list_of_processes(Process* out_processes, unsigned long max);
}
