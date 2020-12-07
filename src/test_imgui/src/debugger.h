
namespace Debugger {

	struct Process {
		unsigned long pid;
		char short_name[50];
	};

	int get_number_registers();
	unsigned long list_of_processes(Process* out_processes, unsigned long max);
}