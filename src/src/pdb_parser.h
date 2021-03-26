namespace PDB {
	void get_pdb_path_from_pe(char* pe_path, char* pdb_path, Debugger::DebuggerData* data);
	void* get_source_to_byte_associations(char* pdb_path);
}
