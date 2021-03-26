// This file is for tools that extract information from Windows Portable Executable (PE) and Program Database (PDB) formats. 

// I'm considering this platform agnostic because we don't need anything windows specific to parse the 
// Windows PE or PDB formats. Linux could also perform this parsing (not that it would be helpful).


// Temporarily use windows file opens/reads/memory allocation until that gets moved to the platform layer

#include <windows.h>
#include "debugger.h"
#include "OSPlatformUI.h"

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

struct MSF {
	unsigned int page_size;
	unsigned int free_block_map;
	unsigned int number_of_pages;
	unsigned int root_stream_size;
	unsigned int reserved;
	unsigned int root_page_number;
};

struct PDBStream {
	unsigned int version;
	unsigned int signature;
	unsigned int age;
	unsigned long guid1;
	unsigned short guid2;
	unsigned short guid3;
	unsigned char guid4[8];
};

struct StreamInfo {
	unsigned int byte_offset_to_block_list;
	unsigned int num_bytes;
	unsigned int num_blocks;
};

struct DbiStreamHeader {
	int version_signature;
	unsigned int version_header;
	unsigned int age;
	unsigned short global_stream_index;
	struct _build_number {
		unsigned short minor_version : 8;
		unsigned short major_version : 7;
		unsigned short new_version_format : 1;
	} build_number;
	unsigned short public_stream_index;
	unsigned short pdb_dll_version;
	unsigned short sym_record_stream;
	unsigned short pdb_dll_rbdl;
	int mod_info_size;
	int section_contribution_size;
	int section_map_size;
	int source_info_size;
	int type_server_map_size;
	unsigned int MFC_type_server_index;
	int optional_dbg_header_size;
	int EC_substream_size;
	struct _flags {
		unsigned short was_inc_linked : 1;
		unsigned short private_symb_stripped : 1;
		unsigned short has_conflicting_types : 1;
		unsigned short reserved : 13;
	} flags;
	unsigned short machine;
	unsigned int padding;
};

struct ModInfo {
	unsigned int unused1;
	struct {
		unsigned short section;
		char padding1[2];
		int offset;
		int size;
		unsigned int characteristics;
		unsigned short module_index;
		char padding2[2];
		unsigned int data_crc;
		unsigned int reloc_crc;
	} SectionContribEntry;
	struct {
		unsigned short dirty : 1;
		unsigned short EC : 1;
		unsigned short unused : 6;
		unsigned short TSM : 8;
	} flags;
	unsigned short module_sym_stream;
	unsigned int sym_byte_size;
	unsigned int c11_byte_size;
	unsigned int c13_byte_size;
	unsigned short source_file_count;
	char padding[2];
	unsigned int unused2;
	unsigned int source_file_name_index;
	unsigned int pdb_file_path_name_index;;
};

struct CVLine {
	unsigned long offset;
	unsigned long linenum_start : 24;
	unsigned long deltaLineEnd : 7;
	unsigned long fStatement : 1; // true if statement, false if expression
};


namespace PDB {
	void get_pdb_path_from_pe(char* pe_path, char* pdb_path, Debugger::DebuggerData *debugger_data) {
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

		debugger_data->base_of_code = pe_optional_64_header.windows_fields.image_base;

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

	void* get_source_to_byte_associations(char* pdb_path) {
		// Just do the file main_ui.cpp for now (for the demo)

		char* main_ui_lines = (char*)OSPlatformUI::allocate_memory(9000); // The source file is only 200 lines long... 9000 bytes should be more than enough
		Debugger::LineNumberData* main_ui_line_number_data = (Debugger::LineNumberData*) main_ui_lines;
		Debugger::LineNumber* line_numbers = (Debugger::LineNumber*) (main_ui_lines+sizeof(Debugger::LineNumberData));
		main_ui_line_number_data->number_of_lines = 0;
		main_ui_line_number_data->file_name[0] = 0;

		// Read in the header and make sure the format is what we expect
		const unsigned long BUFFER_SIZE = 4096;
		char buffer[BUFFER_SIZE];
		unsigned long bytes_read;

		WCHAR exe_wide[DEBUGGER_MAX_PATH];
		for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
			exe_wide[i] = (WCHAR)pdb_path[i];
		}

		HANDLE file_handle = CreateFile(
			exe_wide,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		unsigned long file_size;
		unsigned long high_part;
		file_size = GetFileSize(file_handle, &high_part);

		bool success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);
		if (!success) {
			return NULL;  // Later we should return a error code and pass the line associations via pointer as a parameter 
		}

		// Check for PDB version 7
		char signature[] = "Microsoft C/C++ MSF 7.00\r\nPDS\0\0\0";
		signature[26] = '\x1A';
		for (int i = 0; i < 26; i++) {
			if (buffer[i] != signature[i]) {
				return NULL;  // Meh, should probably ask the user if we want to continue instead
			}
		}

		// Multi-Stream File header
		char* root_buffer = buffer;
		MSF pdb_header = *(MSF*)(buffer + 32);

		// Jump to the root stream "Stream Directory"
		SetFilePointer(file_handle, (pdb_header.root_page_number - 1) * pdb_header.page_size, NULL, FILE_BEGIN);
		success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);
		if (!success) { return NULL; }
		// Stream Directory has form:
		// { unsigned int num_streams,
		//   unsigned int stream_sizes[num_streams]
		//   unsigned int list_of_block_indexes[num_streams][]

		unsigned int num_streams = *(unsigned int*) buffer;
		StreamInfo* stream_info = (StreamInfo*)OSPlatformUI::allocate_memory(sizeof(StreamInfo)*num_streams);


		unsigned int current_block_count = 0;
		for (int i = 0; i < num_streams; i++) {
			stream_info[i].num_bytes = *(unsigned int*)(root_buffer + ((i + 1) * 4));
			stream_info[i].num_blocks = ((stream_info[i].num_bytes + pdb_header.page_size - 1) / pdb_header.page_size);  // Round up to nearest 4096 and then divide byte 4096
			stream_info[i].byte_offset_to_block_list = current_block_count*4 + 4 + num_streams * 4;
			current_block_count += stream_info[i].num_blocks;
		}


		// Read in the PDBInfo stream from index 1
		unsigned int bytes_copied = 0;
		char* pdb_info_buffer = (char*)OSPlatformUI::allocate_memory(stream_info[1].num_blocks * pdb_header.page_size);
		for (int i = 0; i < stream_info[1].num_blocks; i++) {
			unsigned int block = *(unsigned int*)(buffer + stream_info[1].byte_offset_to_block_list + (i * 4));
			SetFilePointer(file_handle, block * pdb_header.page_size, NULL, FILE_BEGIN);
			success = ReadFile(file_handle, (char*)pdb_info_buffer + bytes_copied, pdb_header.page_size, &bytes_read, NULL);
			bytes_copied += bytes_read;
		}

		PDBStream s = *(PDBStream*)(pdb_info_buffer);
		// TODO: double check that the GUID and version match the exe and what we expect

		// Read the Named Stream Map to find which stream /names is stored in
		unsigned long nsm_offset = sizeof(PDBStream);
		unsigned int named_stream_map_size = *(unsigned int*)(pdb_info_buffer + nsm_offset);
		unsigned long count = 0;
		// Count the number of entries in the hash table
		for (int i = nsm_offset + 4; i <= nsm_offset + 4 + named_stream_map_size; i++) {
			if (pdb_info_buffer[i] == 0) {
				count++;
			}
		}

		unsigned int table_offset = nsm_offset + 4 + named_stream_map_size;

		unsigned int table_size = *(unsigned int*)(pdb_info_buffer + table_offset);
		unsigned int table_capacity = *(unsigned int*)(pdb_info_buffer + table_offset + 4);
		unsigned long long present_bit_vector = *(unsigned long long*)(pdb_info_buffer + table_offset + 8) & 0xFFFFFFFFFFFF;
		unsigned long long deleted_bit_vector = *(unsigned long long*)(pdb_info_buffer + table_offset + 14) & 0xFFFFFFFFFFFF;

		unsigned int table_items_offset = table_offset + 20;

		// TODO: I'm not sure how to actually read in the present and deleted bit vectors
		// they should tell us which indexes in the hash table can be read. But instead we're just going
		// to assume that they are in sequential order (which will probably bite us back later

		unsigned int names_stream_index;
		for (int i = 0; i < table_size; i++) {
			unsigned int key = *(unsigned int*)(pdb_info_buffer + table_items_offset + i * 8);
			unsigned int value = *(unsigned int*)(pdb_info_buffer + table_items_offset + i * 8 + 4);
			if (key < BUFFER_SIZE) {
				bool is_names = true;
				for (int i = 0; i < 6; i++) {
					if (pdb_info_buffer[nsm_offset + 4 + key + i] != "/names"[i]) {
						is_names = false;
						break;
					}
				}
				if (is_names) {
					names_stream_index = value;
				}
			}
		}

		char* names_buffer = NULL;
		// Read in the /names string table
		if (names_stream_index != 0) {

			// Read in the PDBInfo stream from index 1
			bytes_copied = 0;
			names_buffer = (char*)OSPlatformUI::allocate_memory(stream_info[names_stream_index].num_blocks * pdb_header.page_size);
			for (int i = 0; i < stream_info[names_stream_index].num_blocks; i++) {
				unsigned int block = *(unsigned int*)(buffer + stream_info[names_stream_index].byte_offset_to_block_list + (i * 4));
				SetFilePointer(file_handle, block * pdb_header.page_size, NULL, FILE_BEGIN);
				success = ReadFile(file_handle, (char*)names_buffer + bytes_copied, pdb_header.page_size, &bytes_read, NULL);
				bytes_copied += bytes_read;
			}
		}


		char* dbi_buffer = (char*)OSPlatformUI::allocate_memory(stream_info[3].num_blocks*pdb_header.page_size);
		bytes_copied = 0;
		for (int i = 0; i < stream_info[3].num_blocks; i++) {
			unsigned int block = *(unsigned int*)(buffer + stream_info[3].byte_offset_to_block_list + (i * 4));
			SetFilePointer(file_handle, block * pdb_header.page_size, NULL, FILE_BEGIN);
			success = ReadFile(file_handle, (char*)dbi_buffer + bytes_copied, pdb_header.page_size, &bytes_read, NULL);
			bytes_copied += bytes_read;
		}

		// Parse the DBI Stream block;
		DbiStreamHeader dbi_stream_header = *(DbiStreamHeader*)(dbi_buffer);


		unsigned int mod_info_offset = sizeof(DbiStreamHeader);
		unsigned long long num_source_files = 0;

		// Parsing modules info 
		char* module_buffer = (char*)OSPlatformUI::allocate_memory(20000);
		unsigned long max_size = 20000;

		unsigned int file_info_offset = dbi_stream_header.mod_info_size
			+ dbi_stream_header.section_contribution_size
			+ dbi_stream_header.section_map_size
			+ sizeof(dbi_stream_header);
		unsigned short num_modules = *(unsigned short*)(dbi_buffer + file_info_offset);
		unsigned int file_names_offset = file_info_offset + 4 + 2 * (num_modules * 2);
		unsigned int num_source_files_2 = 0;
		unsigned int source_count_offset = file_info_offset + 4 + (num_modules * 2);
		//unsigned int file_names_offset = file_info_offset + 4 + 2*(num_modules*2);
		//unsigned int names_buffer_offset = file_names_offset+num_source_files*4;
		for (int i = 0; i < num_modules; i++) {
			num_source_files_2 += *(unsigned short*)(dbi_buffer + source_count_offset + (i * 2));
		}
		unsigned int names_buffer_offset = file_names_offset + num_source_files_2 * 4;
		unsigned int name_count = 0;
		int j = 0;
		while (dbi_buffer[names_buffer_offset + j] != 0) {
			//printf("%i: %i: \t %s\n", name_count, j, stream_buffer + names_buffer_offset + j);
			while (dbi_buffer[names_buffer_offset + j] != '\0') {
				j++;
			}
			j++;
			name_count++;
		}

		while (mod_info_offset < sizeof(DbiStreamHeader) + dbi_stream_header.mod_info_size) {
			ModInfo mod_info = *(ModInfo*)(dbi_buffer + mod_info_offset);
			if (mod_info.module_sym_stream != 0xFFFF) {
				StreamInfo blocks_info = stream_info[mod_info.module_sym_stream];

				// Make sure we have enough memory to store all the blocks
				if (blocks_info.num_blocks * pdb_header.page_size > max_size) {
					OSPlatformUI::free_memory((void*)module_buffer);
					max_size = blocks_info.num_blocks * pdb_header.page_size;
					module_buffer = (char*)OSPlatformUI::allocate_memory(max_size);
				}

				// Clear the blocks
				for (int i = 0; i < max_size; i++) {
					module_buffer[i] = 0xcc;
				}

				// Copy the blocks to the module_buffer
				for (int i = 0; i < blocks_info.num_blocks; i++) {
					unsigned int block = *(unsigned int*)(buffer + blocks_info.byte_offset_to_block_list + (i * 4));
					SetFilePointer(file_handle, block * pdb_header.page_size, NULL, FILE_BEGIN);
					success = ReadFile(file_handle, (char*)module_buffer + i * pdb_header.page_size, pdb_header.page_size, &bytes_read, NULL);
				}

				// Parse C13 line information 
				unsigned int line_information_offset = mod_info.sym_byte_size;
				unsigned int checksum_offset = 0;
				unsigned int name_offset = 0;
				while (line_information_offset - mod_info.sym_byte_size < mod_info.c13_byte_size) {
					unsigned int info_type = *(unsigned int*)(module_buffer + line_information_offset);
					unsigned int size = *(unsigned int*)(module_buffer + line_information_offset + 4);
					if (info_type != 0xf2) { // DEBUG_S_LINES
						if (info_type == 0xf4) {
							checksum_offset = line_information_offset;
						}
					}
					else {
						unsigned int base_address = *(unsigned int*)(module_buffer + line_information_offset + 8);
						unsigned short section_number = *(unsigned short*)(module_buffer + line_information_offset + 12);
						unsigned short flags = *(unsigned short*)(module_buffer + line_information_offset + 14);
						if (flags & 0x0001) {
							//printf("Line numbers have column data\n");
						}
						unsigned int range = *(unsigned int*)(module_buffer + line_information_offset + 16);
						unsigned int offset_file = *(unsigned int*)(module_buffer + line_information_offset + 20);
						name_offset = *(unsigned int*)(module_buffer + checksum_offset + offset_file + 8) + 12;
						unsigned int num_pairs = *(unsigned int*)(module_buffer + line_information_offset + 24);
						unsigned int bytes_of_code = *(unsigned int*)(module_buffer + line_information_offset + 28);
						//printf("Offset into names buffer: %i, module name: %s\n", name_offset, stream_buffer+mod_info_offset+sizeof(ModInfo));
						//printf("File name: %s\n", names_buffer + name_offset);
						
						// If the file name has "main_ui.cpp" in it: 
						int j = 0;
						char source_name[] = "main_ui.cpp\0";
						bool match = false;
						for (int i = name_offset; names_buffer[i] != 0; i++) {
							if (names_buffer[i] == source_name[j]) {
								j++;
							}
							else {
								j = 0;
							}
							if (j == 10) {
								match = true;
							}
						}
						if (match) {
							if (main_ui_line_number_data->file_name[0] == 0) {

								int i = name_offset;
								for (; names_buffer[i] != 0; i++) {
									main_ui_line_number_data->file_name[i - name_offset] = names_buffer[i];
								}
								main_ui_line_number_data->file_name[i - name_offset] = 0;
							}
							OutputDebugStringA((LPCSTR) (names_buffer + name_offset));
							OutputDebugStringA((LPCSTR) "\n");
							for (int i = 0; i < num_pairs; i++) {
								CVLine line_info = *(CVLine*)(module_buffer + line_information_offset + 32 + 8 * i);
								line_numbers[main_ui_line_number_data->number_of_lines].offset_into_section = line_info.offset + base_address;
								line_numbers[main_ui_line_number_data->number_of_lines].line_number = line_info.linenum_start;
								main_ui_line_number_data->number_of_lines += 1;
							}
						}
					}
					line_information_offset += size + 8;
				}

			}

			mod_info_offset += sizeof(ModInfo);
			//printf("\t\t\t\t\tIndex: %i\n", mod_info.SectionContribEntry.module_index);
			// It's possible that the next 16 bits are 0, and the Module Name starts right after that...
			if (*(unsigned short*)(dbi_buffer + mod_info_offset) == 0) {
				mod_info_offset += 2;
			}
			num_source_files += mod_info.source_file_count;
			// Find next string
			//printf("Module Name: %s\n", dbi_buffer + mod_info_offset);
			if (dbi_buffer[mod_info_offset] == 0) {
				mod_info_offset++;
			}
			while (dbi_buffer[mod_info_offset] != 0) {
				mod_info_offset++;
			}
			mod_info_offset++;
			//printf("ObjFileName: %s\n", stream_buffer + mod_info_offset);
			if (dbi_buffer[mod_info_offset] == 0) {
				mod_info_offset++;
			}
			// Go to the next ModInfo offset 
			while (dbi_buffer[mod_info_offset] != 0) {
				mod_info_offset++;
			}
			mod_info_offset++;
			// Align forward to a 4 byte alignment
			mod_info_offset = (mod_info_offset + 4 - 1) & ~(4 - 1);
		}
		/*

		//printf("Parse File Info Substream:\n");
		unsigned int file_info_offset = dbi_stream_header.mod_info_size
			+ dbi_stream_header.section_contribution_size
			+ dbi_stream_header.section_map_size
			+ sizeof(dbi_stream_header);
		unsigned short num_modules = *(unsigned short*)(dbi_buffer + file_info_offset);
		//printf("Number of modules: %i\n", num_modules);
		//printf("Number of source files: %i\n", num_source_files);
		//printf("Module Indicies = {");
		for (int i = 0; i < num_modules; i++) {
			//printf("%i, ", *(unsigned short*)(dbi_buffer + file_info_offset + 4 + (i * 2)));
		}
		//printf("}\n");
		//unsigned int num_source_files_2 = 0;
		num_source_files_2 = 0;
		//unsigned int source_count_offset = file_info_offset + 4 + (num_modules*2);
		//unsigned int file_names_offset = file_info_offset + 4 + 2*(num_modules*2);
		//unsigned int names_buffer_offset = file_names_offset+num_source_files*4;
		for (int i = 0; i < num_modules; i++) {
			unsigned short this_mod_src_count = *(unsigned short*)(dbi_buffer + source_count_offset + (i * 2));
			num_source_files_2 += this_mod_src_count;
			//printf("Mod %i has file count %i\n", i, this_mod_src_count);
		}
		//printf("New source file count: %i\n", num_source_files_2);
		for (int i = 0; i < num_source_files; i++) {
			unsigned int file_name_offset = *(unsigned int*)(dbi_buffer + file_names_offset + (i * 4));
			//printf("Source file name: %i, %i, %s\n", file_name_offset, i, stream_buffer + names_buffer_offset + file_name_offset);
		}
		//printf("\n");
		*/
		return (void*)main_ui_lines;
	}
}
