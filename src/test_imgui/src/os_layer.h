namespace OSLayer {

	int open_file(char* returned_file_path);
	void* allocate_memory(unsigned long long size_bytes);
	bool free_memory(void* memory);

}