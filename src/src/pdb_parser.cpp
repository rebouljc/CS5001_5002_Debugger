// This file is for tools that extract information from Windows Portable Executable (PE) and Program Database (PDB) formats. 

// I'm considering this platform agnostic because we don't need anything windows specific to parse the 
// Windows PE or PDB formats. Linux could also perform this parsing (not that it would be helpful).


// Temporarily use windows file opens/reads/memory allocation until that gets moved to the platform layer

#include <windows.h>
HANDLE open_file(char* path) {
	// Super basic file open API for reading only 
	WCHAR path_wide[MAX_PATH];
	for (int i = 0; i < MAX_PATH; i++) {
		path_wide[i] = path[i];
	}

	HANDLE file_handle = CreateFile(
		path_wide,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return file_handle;
}

unsigned long read_file(HANDLE file_handle, char* buffer, unsigned int num_bytes) {
	unsigned long bytes_read;
	bool success = ReadFile(file_handle, buffer, num_bytes, &bytes_read, NULL);  // Maybe we should handle the errors here?
	return bytes_read;
}

unsigned long read_file(HANDLE file_handle, char* buffer, unsigned int num_bytes, unsigned long from) {
	SetFilePointer(file_handle, from, NULL, FILE_BEGIN);
	unsigned long bytes_read;
	bool success = ReadFile(file_handle, buffer, num_bytes, &bytes_read, NULL);  // Maybe we should handle the errors here?
	return bytes_read;
}

bool close_file(HANDLE file_handle) {
	return CloseHandle(file_handle);
}


struct COFF_HEADER {
	         char  signature[4];              // Should be "PE\0\0"
	unsigned short machine;                   // signature that identifies machine type
	unsigned short number_of_sections;        // 
	unsigned int   time_date_stamp;           // the low 32 bits of epoch time in seconds
	unsigned int   pointer_to_symbol_table;   // should be 0 - depracated  
	unsigned int   number_of_symbols;         // should be 0 - deprecated 
	unsigned short size_of_optional_header;   // size in bytes
	unsigned short characteristics;           // info about the exe/obj file
};

struct PE64_OP_HEADER {
	struct {
		unsigned short magic;
		unsigned char  major_linker_version;
		unsigned char  minor_linker_version;
		unsigned int   size_of_code;
		unsigned int   size_of_initialized_data;
		unsigned int   size_of_unitialized_data;
		unsigned int   address_of_entry_point;
		unsigned int   base_of_code;
	} standard_fields;

	struct {
		unsigned long long  image_base;
		unsigned int   section_alignement;
		unsigned int   file_alignment;
		unsigned short major_operating_system_version;
		unsigned short minor_operating_system_version;
		unsigned short major_image_version;
		unsigned short minor_image_version;
		unsigned short major_subsystem_version;
		unsigned short minor_subsystem_version;
		unsigned int   win32_version_value;
		unsigned int   size_of_image;
		unsigned int   size_of_headers;
		unsigned int   checksum;
		unsigned short subsystem;
		unsigned short dll_characteristics;
		unsigned long  size_of_stack_reserve;
		unsigned long  size_of_stack_commit;
		unsigned long  size_of_heap_reserve;
		unsigned long  size_of_heap_commit;
		unsigned int   loader_flags;
		unsigned int   number_of_rva_and_sizes;
	} windows_fields;
};

struct DATA_DIR {
	unsigned int virtual_address;
	unsigned int size;
};

struct SECTION_HEADER {
	         char  name[8];
	unsigned int   virtual_size;               // size of the section when loaded into memory
	unsigned int   virtual_address;            // address of the first byte of the section when loaded into memory (releative to image base)
	unsigned int   size_of_raw_data;           // size of initialized data on disk
	unsigned int   pointer_to_raw_data;        // file pointer to the first page of the section (multiple of file_alignment)
	unsigned int   pointer_to_relocations;     // zero for executables 
	unsigned int   pointer_to_line_numbers;    // deprecated (COFF debugging info is deprecated)
	unsigned short number_of_relocations;      // zero for executables
	unsigned short number_of_line_numbers;     // deprecated (COFF debugging info is deprecated)
	unsigned int   characteristics;            // info about the section https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#section-flags
};


/*
struct GUID {
	unsigned long  data1;    // first 8 hex digits
	unsigned short data2;    // first group of 4 hex digits
	unsigned short data3;    // second group of 4 hex digits
	unsigned char  data4[8]; // first 2 bytes contin the third group, last 6 bytes is the final group
};
*/

struct CODEVIEW_HEADER {
	unsigned char signature[4];
	         GUID guid;
    unsigned int  age;
//           char pdb_path[] 
};

struct DEBUG_DIR_HEADER {
	unsigned int   characteristics;      // reserved, must be 0
	unsigned int   time_date_stamp;      // time and date when created
	unsigned short major_version;        
	unsigned short minor_version;
	unsigned int   type;                 // The format of the debugging information
	unsigned int   size_of_data;         // size of the debug data (not including the debug dir itself)
	unsigned int   address_of_raw_data;  // address of the debug data when loaded, relative to the image base
	unsigned int   pointer_to_raw_data;  // the file pointer to the debug data
};

namespace PDB {
	void get_pdb_path_from_pe(char* pe_path, char* pdb_path) {
		const unsigned long BUFFER_SIZE = 2048;
		char buffer[BUFFER_SIZE];
		
		// TODO: At some point we should check that the file is actually a PE file
		//       which is done by checking that the first two bytes are "MZ"
		HANDLE pe_file = open_file(pe_path);
		read_file(pe_file, buffer, BUFFER_SIZE);
		
		unsigned short pe_header_offset = *(unsigned short*)(buffer + 0x3C);

		read_file(pe_file, buffer, BUFFER_SIZE, pe_header_offset);
		COFF_HEADER coff_header = *(COFF_HEADER*)(buffer);

		PE64_OP_HEADER pe_optional_64_header;

		// If the optional header is actually 32 bit, copy the values into the 64 bit version
		bool pe_is_64 = *(unsigned short*)(buffer + sizeof(COFF_HEADER)) == 0x20b;

		if (pe_is_64) {
			pe_optional_64_header = *(PE64_OP_HEADER*)(buffer + sizeof(COFF_HEADER));
		}
		else {
			// Copy the first 24 bytes directly, as they're the same
			for(int i = 0; i < 24; i++){
				((unsigned char*)(&pe_optional_64_header))[i] = buffer[sizeof(COFF_HEADER)+i];
			}
			pe_optional_64_header.windows_fields.image_base = ((unsigned short*)(buffer+sizeof(COFF_HEADER)))[28];
			// Copy from bytes 32 to 72, they are also the same
			for(int i = 32; i < 72; i++){
				((unsigned char*)(&pe_optional_64_header))[i] = buffer[sizeof(COFF_HEADER)+i];
			}
			pe_optional_64_header.windows_fields.size_of_stack_reserve   = *(unsigned int*)(buffer + 72 + sizeof(COFF_HEADER));
			pe_optional_64_header.windows_fields.size_of_stack_commit    = *(unsigned int*)(buffer + 76 + sizeof(COFF_HEADER));
			pe_optional_64_header.windows_fields.size_of_heap_reserve    = *(unsigned int*)(buffer + 80 + sizeof(COFF_HEADER));
			pe_optional_64_header.windows_fields.size_of_heap_commit     = *(unsigned int*)(buffer + 84 + sizeof(COFF_HEADER));
			pe_optional_64_header.windows_fields.loader_flags            = *(unsigned int*)(buffer + 88 + sizeof(COFF_HEADER));
			pe_optional_64_header.windows_fields.number_of_rva_and_sizes = *(unsigned int*)(buffer + 92 + sizeof(COFF_HEADER));
		}


		if (coff_header.signature[0] != 'P' || coff_header.signature[1] != 'E') {
			OutputDebugString(L"PE signature did not match");
		}
		else {
			OutputDebugString(L"PE signature did match");
		}

		// This will tell us where the debug_directory will be located in the executable
		// when loaded into memory. We need to figure out what section it's in, then 
		// use the physical file location of the section to find the physical location
		// of the debug directory.
		DATA_DIR debug_directory_info = *(DATA_DIR*)(buffer + sizeof(COFF_HEADER) + coff_header.size_of_optional_header - 80);
		unsigned int debug_dir_file_offset;

		for (int i = 0; i < coff_header.number_of_sections; i++) {
			SECTION_HEADER section_header = *(SECTION_HEADER*)(buffer + (i * sizeof(SECTION_HEADER)) + sizeof(COFF_HEADER) + coff_header.size_of_optional_header);
			if (debug_directory_info.virtual_address > section_header.virtual_address
				&& debug_directory_info.virtual_address < (section_header.virtual_address + section_header.virtual_size)) {

				debug_dir_file_offset = debug_directory_info.virtual_address - (section_header.virtual_address - section_header.pointer_to_raw_data);
			}
		}


		// Jump to the debug data directory:
		read_file(pe_file, buffer, BUFFER_SIZE, debug_dir_file_offset);
		DEBUG_DIR_HEADER debug_dir_header = *(DEBUG_DIR_HEADER*)(buffer);

		if (debug_dir_header.type != 0x02) {
			OutputDebugString(L"Not CodeView!!!");
			int* x = NULL;
			*x = 0;
		}

		// jump to the codeview debug data
		read_file(pe_file, buffer, BUFFER_SIZE, debug_dir_header.pointer_to_raw_data);

		// Probably want to save this codeview_header somewhere so we can verify
		// that the exe and pdb files match
		CODEVIEW_HEADER codeview_header = *(CODEVIEW_HEADER*)(buffer);
		for (int i = 0; i < debug_dir_header.size_of_data; i++) {
			pdb_path[i] = buffer[i+sizeof(CODEVIEW_HEADER)];
		}

		close_file(pe_file);
	}
}
