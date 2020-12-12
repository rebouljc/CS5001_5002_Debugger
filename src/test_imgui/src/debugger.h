
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
		bool running = false;
		bool debugging = false;
		unsigned long pid = 0;
		unsigned long tid = 0;
	};

	struct Process {
		unsigned long pid;
		char short_name[50];
	};

	int debug_init(DebuggerData* data);
	int debug_loop(DebuggerData* data);
	
	int start_and_debug_exe(DebuggerData* data);
	int get_number_registers();
	unsigned long list_of_processes(Process* out_processes, unsigned long max);
}