// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
//#define DIRECTINPUT_VERSION 0x0602 
//#include <dinput.h>
#include <tchar.h>
#include <stdio.h>

#include "main_ui.h"
#include <os_layer.h>

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct data_dir {
    unsigned long virtual_address;
    unsigned long size;
};

struct COFF {
    unsigned short machine;
    unsigned short number_of_sections;
    unsigned int time_date_stamp;
    unsigned int pointer_to_symbol_table;
    unsigned int number_of_symbols;
    unsigned short size_of_optional_header;
    unsigned short characteristics;
};

struct PEOP {
    unsigned short magic;
    unsigned char major_linker_version;
    unsigned char minor_linker_version;
    unsigned int size_of_code;
    unsigned int size_of_initialized_data;
    unsigned int size_of_uninitialized_data;
    unsigned int address_of_entry_point;
    unsigned int base_of_code;
};

struct DEBUG_DIR {
    unsigned int characteristics;
    unsigned int time_date_stamp;
    unsigned short major_version;
    unsigned short minor_version;
    unsigned int type;
    unsigned int size_of_data;
    unsigned int address_of_raw_data;
    unsigned int pointer_to_raw_data;
};

struct CODEVIEW {
    unsigned char signature[4];
	unsigned long guid1;
	unsigned short guid2;
	unsigned short guid3;
	unsigned char guid4[8];
    unsigned int age;
};

struct SECTION {
    char name[8];
    unsigned int virtual_size;
    unsigned int virtual_address;
    unsigned int size_of_raw_data;
    unsigned int pointer_to_raw_data;
    unsigned int pointer_to_relocations;
    unsigned int pointer_to_line_numbers;
    unsigned short number_of_relocations;
    unsigned short number_of_line_numbers;
    unsigned int characteristics;
};

struct MSF {
    // All the values in the PDB file are in little endian format
    unsigned int page_size;
    unsigned int free_block_map;
    unsigned int number_of_pages;
    unsigned int root_stream_size;
    unsigned int reserved;
    unsigned int root_page_number;
};

struct MyGUID {
    unsigned char signature[4];
    unsigned long guid1;
    unsigned short guid2;
    unsigned short guid3;
    unsigned char guid4[8];
};

struct CVLine {
    unsigned long offset;
    unsigned long linenum_start : 24;
    unsigned long deltaLineEnd: 7;
    unsigned long fStatement: 1; // true if statement, false if expression
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

enum PdbRaw_FeatureSig {
    VC110 = 20091201,
    VC140 = 20140508,
    NoTypeMerge = 0x4D544F4E,     // NOTM
    MinimalDebugInfo = 0x494E494D // MINI
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

struct DbiStreamHeader {
    int version_signature;
    unsigned int version_header;
    unsigned int age;
    unsigned short global_stream_index;
    struct _build_number {
        unsigned short minor_version : 8;
        unsigned short major_version: 7;
        unsigned short new_version_format: 1;
    } build_number;
    unsigned short public_stream_index;
    unsigned short pdb_dll_vesrion;
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
		unsigned short was_inc_linked:1;
		unsigned short private_symb_stripped:1;
		unsigned short has_conflicting_types:1;
		unsigned short reserved:13;
    } flags;
    unsigned short machine;
    unsigned int padding;
};

void get_pdb_file_path_test(char* exe_path, char* output_path) {

	WCHAR exe_wide[DEBUGGER_MAX_PATH];
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		exe_wide[i] = (WCHAR)exe_path[i];
	}

    HANDLE file_handle;
    file_handle = CreateFile(
        exe_wide,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL 
    );    


    char buffer[2048];
    unsigned long bytes_read = 0;

    bool success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
    printf("bytes read: %i\n", bytes_read);

    unsigned int pe_offset = ((*(unsigned short*)(buffer + 0x3C)) & 0xFFFF) + 4;
    unsigned int pe_op_offset = pe_offset + 20;

    COFF coff_data = *(COFF*)(buffer + pe_offset);
    PEOP op_standard_entries = *(PEOP*)(buffer+pe_op_offset);

    printf("pe_offset: %i\n", pe_offset);
    //unsigned int pe_offset = 5;
    // + 4 to account for PE\0\0

    unsigned int section_start = pe_op_offset + coff_data.size_of_optional_header;

	printf("Debug Data stripped flag: %i\n", coff_data.characteristics & 0x0200);
    printf("Optional header size: %i\n", coff_data.size_of_optional_header);
    printf("Number of sections: %i\n", coff_data.number_of_sections);
    //printf("Characteristics hex : %#02x\n", characteristics);
    printf("PEOP magic number: %#08x\n", op_standard_entries.magic);
    unsigned int rva_offset = 0;
    unsigned int data_dirs_offset;
    unsigned int debug_data_dir_offset;

    if (op_standard_entries.magic == 0x10b) {
		printf("PE32 format\n");
		rva_offset = pe_op_offset + 92;
		data_dirs_offset = pe_op_offset + 96;
        debug_data_dir_offset = pe_op_offset + 144;
    }
    else if (op_standard_entries.magic == 0x20b) {
		printf("PE32+ format\n");
		rva_offset = pe_op_offset + 108;
		data_dirs_offset = pe_op_offset + 112;
        debug_data_dir_offset = pe_op_offset + 160;
    }
    else if (op_standard_entries.magic == 0x107) {
		printf("ROM format\n");
    }

    unsigned int num_rva_sizes = *(unsigned int*)(buffer + rva_offset) & 0xFFFFFFFF;
    printf("NumberOfRvaAndSizes: %i\n", num_rva_sizes);

    data_dir debug_dir = *(data_dir*)(buffer + debug_data_dir_offset);
    printf("\tDebug data VirtualAddress: %#08X Size: %#08X", debug_dir.virtual_address, debug_dir.size);
    unsigned long debug_dir_file_offset;

    printf("Sections: \n");

    int current_section = 0;
    char name[9] = { 0 };
    for (int i = 0; i < coff_data.number_of_sections; i++) {
        SECTION current_section = *(SECTION*)(buffer + section_start + i * sizeof(SECTION));

        for (int x = 0; x < 8; x++) {
            name[x] = current_section.name[x];
        }
        

        //if (current_section.characteristics == (IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA)) {
            printf("\t%s\t%#08X\t%i\t%#08X\n", name, current_section.pointer_to_raw_data, current_section.characteristics & IMAGE_SCN_CNT_CODE, current_section.pointer_to_raw_data+current_section.size_of_raw_data);
            printf("\t\tCharacteristics: %#08X\n", current_section.characteristics);
        //}
        if ((debug_dir.virtual_address < (current_section.virtual_address + current_section.virtual_size))
            && (debug_dir.virtual_address > current_section.virtual_address)) {
            debug_dir_file_offset = debug_dir.virtual_address- ((current_section.virtual_address) - (current_section.pointer_to_raw_data));
            printf("\t\t\tDebug dir file offset: %#08X\n", debug_dir_file_offset);
        }
    }

    unsigned long current_start = 0;
    unsigned long size = 2048;

    SetFilePointer(file_handle, debug_dir_file_offset, NULL, FILE_BEGIN);
	success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
	if (!success) {
		printf("Reached end of file?\n");
		return;
	}
    DEBUG_DIR another_debug_dir = *(DEBUG_DIR*)(buffer);
    
    if (another_debug_dir.type == 2) {
        printf("Debug data is Visual C++ debug information\n");
        printf("Major version: %i Minor Version %i\n", another_debug_dir.major_version, another_debug_dir.minor_version);
    }


    SetFilePointer(file_handle, another_debug_dir.pointer_to_raw_data, NULL, FILE_BEGIN);
	success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
	if (!success) {
		printf("Reached end of file?\n");
		return;
	}

    CODEVIEW codeview = *(CODEVIEW*)(buffer);
    printf("\tcodeview header:\n");
    printf("\t\tSignature: ");
    for (int i = 0; i < 4; i++) {
        printf("%c", codeview.signature[i]);
    }

    printf("\n\t\tGUID: ");
    printf("%#04x - ", codeview.guid1);
    printf("%#02x - ", codeview.guid2);
    printf("%#02x - ", codeview.guid3);
    for (int i = 0; i < 8; i++) {
        printf(" %#01x ", codeview.guid4[i]);
    }
    printf("\n");

    printf("\t\tage: %i\n",codeview.age);

    printf("\t\tFile Path: %s\n", (buffer+sizeof(codeview)));

	char* pdb_file_path = (buffer + sizeof(codeview));

	int i = 0;
	while (i < DEBUGGER_MAX_PATH && pdb_file_path[i] != 0) {
		output_path[i] = pdb_file_path[i];
		i++;
	}

    CloseHandle(file_handle);

}

void parse_pdb_file(char* pdb_file_path) {
    const unsigned long BUFFER_SIZE = 4096;
	WCHAR exe_wide[DEBUGGER_MAX_PATH];
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		exe_wide[i] = (WCHAR)pdb_file_path[i];
	}

    HANDLE file_handle;
    file_handle = CreateFile(
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
    printf("\n");
    printf("Opened pdb file of size %i\n", file_size);

    char buffer[BUFFER_SIZE];
    unsigned long bytes_read = 0;

    bool success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);

    // Check the for the PDB version 7 signature. Otherwise, it's probably
    // a PDB version 2 (or not a PDB file).
    char signature[] = "Microsoft C/C++ MSF 7.00\r\nPDS\0\0\0";
    signature[26] = '\x1A';
    bool version7 = true;
    for (int i = 0; i < 32; i++) {
        if (buffer[i] != signature[i]) {
            version7 = false;
            printf("PDB did not match version 7 signature\n");
		    break;
        }
    }

    if (version7) {
        printf("PDB matched version 7 signature\n");
    }

    MSF pdb_header = *(MSF*)(buffer+32);


    printf("\tpage_size: %i\n", pdb_header.page_size);
    printf("\tfree_block_map: %i\n", pdb_header.free_block_map);
    printf("\tnumber_of_pages: %i\n", pdb_header.number_of_pages);
    printf("\troot_stream_size: %i\n", pdb_header.root_stream_size);
    printf("\treserved: %i\n", pdb_header.reserved);
    printf("\troot_page_number: %i\n", pdb_header.root_page_number);
    printf("\tfile size should be: %i\n", pdb_header.page_size*pdb_header.number_of_pages);

    // Read until root page:
    // TODO: currently we're assuming the block size is 4096 via llvm documentation
    for (int i = 0; i < pdb_header.root_page_number-1; i++) {
		success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);
    }

    // Should be at the root page now:
    unsigned int num_streams = *(unsigned int*)(buffer);
    printf("\tnumber of streams: %i\n", num_streams);
    
    unsigned int current_stream_size = 0;
    unsigned int stream_block_size = 0;
    unsigned int total_blocks = 0;
    unsigned int block;
    

    unsigned int pdb_header_block = 0;
    unsigned int pdb_stream_size = 0;

    for (int i = 0; i < num_streams; i++) {
        current_stream_size = *(unsigned int*)(buffer + ((i + 1) * 4));
        stream_block_size = ((current_stream_size+pdb_header.page_size-1)/pdb_header.page_size);
        printf("\t\tStream size %i: %i", i, current_stream_size);
        printf("\t\t\tblocks: %i\n", stream_block_size);
        printf("\t\t\tblocks: {");
        for (int b = total_blocks; b < total_blocks + stream_block_size; b++) {
            block = *(unsigned int*)(buffer+(num_streams*4)+((b+1)*4));
            if (i == 0) {
                pdb_header_block = block;
				pdb_stream_size = current_stream_size;
            }
            printf("%i, ", block);
        }
        printf("}\n");
        total_blocks += stream_block_size;
    }

    printf("\ttotal counted blocks: %i\n", total_blocks);
    printf("\n");

    printf("\tNow reading the PDB stream from block %i\n", pdb_header_block);

	// Go to the pdb stream 
    unsigned long ret = SetFilePointer(file_handle, (pdb_header_block-1)*pdb_header.page_size, NULL, FILE_BEGIN);
    success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);

    printf("\t\tPDB Stream size: %i\n", pdb_stream_size);
    PDBStream s = *(PDBStream*)(buffer);

    if (s.version == 20000404) {
        printf("\t\tGot VC70\n");
    }
    else {
        printf("\t\tGot a different version number!!!: %i\n", s.version);
    }

    // We can use the pdb GUID and compare it to the exe GUID to make sure they are matching
    printf("\n\t\tGUID: ");
    printf("%#04x - ", s.guid1);
    printf("%#02x - ", s.guid2);
    printf("%#02x - ", s.guid3);
    for (int i = 0; i < 8; i++) {
        printf(" %#01x ", s.guid4[i]);
    }
    printf("\n");


    unsigned long nsm_offset = sizeof(PDBStream);
    printf("\tReading the Named Stream Map\n");
    unsigned int named_stream_map_size = *(unsigned int*)(buffer+nsm_offset);
    unsigned long count = 0;
    // Count the number of entries in the has table
    for (int i = nsm_offset+4; i <= nsm_offset + 4+ named_stream_map_size; i++) {
        if (buffer[i] == 0) {
            count++;
        }
    }

    unsigned int table_offset = nsm_offset + 4 + named_stream_map_size;

    unsigned int table_size         = *(unsigned int*)(buffer + table_offset);
    unsigned int table_capacity     = *(unsigned int*)(buffer + table_offset+4);
    unsigned long long present_bit_vector = *(unsigned long long*)(buffer + table_offset+8) & 0xFFFFFFFFFFFF;
    unsigned long long deleted_bit_vector = *(unsigned long long*)(buffer + table_offset+14) & 0xFFFFFFFFFFFF;

    unsigned int table_items_offset = table_offset + 20;

    printf("\tsize: %i\n", table_size);
    printf("\tcapacity: %i\n", table_capacity);
    // TODO: I'm not sure how to actually read in the present and deleted bit vectors
    // they should tell us which indexes in the hash table can be read. But instead we're just going
    // to assume that they are in sequential order (which will probably bite us back later
    printf("\tpresent: %I64u\n", present_bit_vector);
    printf("\tdeleted: %I64u\n", deleted_bit_vector);

    printf("\tvalues:\n");
    for (int i = 0; i < table_size; i++) {
        unsigned int key = *(unsigned int*)(buffer+table_items_offset+i*8);
        unsigned int value = *(unsigned int*)(buffer+table_items_offset+i*8+4);
        if (key < BUFFER_SIZE) {
			printf("\t\tkey: %i\tvalue: %i\tstring:%s\n", key, value, buffer+nsm_offset+4+key);
        }
        else {
			printf("\t\tkey: %i\tvalue: %i\n", key, value);
        }
    }

    unsigned int feature_sig_offset = table_items_offset+table_size*8;
    // TODO: How do I know where to start and when to stop reading the list?
    // llvm says that the enums fill the rest of the PDB Stream size, but the size is about 2k,
    // even though the PDB Stream stops giving relevant information after many less bytes
    for (int i = 0; i < 10; i+=1) {
        unsigned int feature_sig = *(unsigned int*)(buffer+feature_sig_offset+(i*4));
        if (feature_sig == 0) {
            continue;
        }

        printf("\tfeature: ");
        if (feature_sig == VC110) {
            printf("VC100");
        } else if (feature_sig == VC140) {
            printf("VC140");
        } else if (feature_sig == NoTypeMerge) {
            printf("NoTypeMerge");
        } else if (feature_sig == MinimalDebugInfo) {
            printf("MinimalDebugInfo");
        }
        printf("\n");
    }
	printf("\n");


    // Iterate through and print out the values for the hash table
    /*
    for (int i = 0; i < table_size; i++) {

        unsigned int key = *(unsigned int*)(buffer + (i*4));
        unsigned int value_page = *(unsigned int*)(buffer + i + 1);
        printf("%s is in page %i\n", buffer+key, value_page);
    }
    */

    printf("\n");


    CloseHandle(file_handle);
    printf("Closed pdb file\n");
}


struct StreamBlockInfo {
    unsigned int byte_offset_to_blocks;
    unsigned int num_bytes;
    unsigned int num_blocks;
};


void parse_pdb_file_2(char* pdb_file_path) {
    const unsigned long BUFFER_SIZE = 4096;
	WCHAR exe_wide[DEBUGGER_MAX_PATH];
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		exe_wide[i] = (WCHAR)pdb_file_path[i];
	}

    HANDLE file_handle;
    file_handle = CreateFile(
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
    printf("\n");
    printf("Opened pdb file of size %i\n", file_size);

    char buffer[BUFFER_SIZE];
    unsigned long bytes_read = 0;

    bool success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);

    // Check the for the PDB version 7 signature. Otherwise, it's probably
    // a PDB version 2 (or not a PDB file).
    char signature[] = "Microsoft C/C++ MSF 7.00\r\nPDS\0\0\0";
    signature[26] = '\x1A';
    bool version7 = true;
    for (int i = 0; i < 32; i++) {
        if (buffer[i] != signature[i]) {
            version7 = false;
            printf("PDB did not match version 7 signature\n");
		    break;
        }
    }

    if (version7) {
        printf("PDB matched version 7 signature\n");
    }

    MSF pdb_header = *(MSF*)(buffer+32);


    printf("\tpage_size: %i\n", pdb_header.page_size);
    printf("\tfree_block_map: %i\n", pdb_header.free_block_map);
    printf("\tnumber_of_pages: %i\n", pdb_header.number_of_pages);
    printf("\troot_stream_size: %i\n", pdb_header.root_stream_size);
    printf("\treserved: %i\n", pdb_header.reserved);
    printf("\troot_page_number: %i\n", pdb_header.root_page_number);
    printf("\tfile size should be: %i\n", pdb_header.page_size*pdb_header.number_of_pages);

    // Jump to the root stream
    SetFilePointer(file_handle, (pdb_header.root_page_number-1)*pdb_header.page_size, NULL, FILE_BEGIN);
	success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);

    // Should be at the root stream now:
    unsigned int num_streams = *(unsigned int*)(buffer);
    printf("\tnumber of streams: %i\n", num_streams);

    StreamBlockInfo *stream_block_info = (StreamBlockInfo*)OSLayer::allocate_memory(sizeof(StreamBlockInfo)*num_streams);
    
    unsigned int current_stream_size = 0;
    unsigned int stream_block_size = 0;
    unsigned int total_blocks = 0;
    unsigned int block;
    

    unsigned int pdb_header_block = 0;
    unsigned int pdb_stream_size = 0;

    char* stream_buffer = NULL; 

    unsigned int bytes_copied = 0;
    char* names_buffer = NULL; 
    unsigned int names_stream_index = 0;
    char* pdb_info_buffer = NULL;

    for (int i = 0; i < num_streams; i++) {
        current_stream_size = *(unsigned int*)(buffer + ((i + 1) * 4));
        stream_block_size = ((current_stream_size+pdb_header.page_size-1)/pdb_header.page_size);
        printf("\t\tStream size %i: %i", i, current_stream_size);
        printf("\t\t\tblocks: %i\n", stream_block_size);
        printf("\t\t\tblocks: {");

        stream_block_info[i].byte_offset_to_blocks = (num_streams * 4) + ((total_blocks + 1) * 4);
        stream_block_info[i].num_bytes = current_stream_size;
        stream_block_info[i].num_blocks = stream_block_size;

	   // TODO create a custom memory allocation system that is OS independent.
        if (i == 3) {
            stream_buffer = (char*)VirtualAlloc(NULL, current_stream_size, MEM_COMMIT, PAGE_READWRITE);
        }
		for (int b = total_blocks; b < total_blocks + stream_block_size; b++) {
			block = *(unsigned int*)(buffer+(num_streams*4)+((b+1)*4));

			if (i == 3) {
				SetFilePointer(file_handle, block*pdb_header.page_size, NULL, FILE_BEGIN);
				success = ReadFile(file_handle, (char*)stream_buffer+bytes_copied, pdb_header.page_size, &bytes_read, NULL);
				printf("Copied block %i into the stream buffer at byte %i \n", block, bytes_copied);
				bytes_copied += bytes_read;
			}
			
			if (i == 0) {
				pdb_header_block = block;
				pdb_stream_size = current_stream_size;
			}

			printf("%i, ", block);
		}

        printf("}\n");
        total_blocks += stream_block_size;
    }

    // Read in the PDBInfo stream from index 1
    bytes_copied = 0;
    pdb_info_buffer = (char*)OSLayer::allocate_memory(stream_block_info[1].num_bytes);
    for (int i = 0; i < stream_block_info[1].num_blocks; i++) {
        block = *(unsigned int*)(buffer+stream_block_info[1].byte_offset_to_blocks + (i*4));
		SetFilePointer(file_handle, block*pdb_header.page_size, NULL, FILE_BEGIN);
		success = ReadFile(file_handle, (char*)pdb_info_buffer+bytes_copied, pdb_header.page_size, &bytes_read, NULL);
        bytes_copied += bytes_read;
    }

    // Read the Named Stream Map to find which stream /names is stored in
    PDBStream s = *(PDBStream*)(pdb_info_buffer);

    unsigned long nsm_offset = sizeof(PDBStream);
    printf("\tReading the Named Stream Map\n");
    unsigned int named_stream_map_size = *(unsigned int*)(pdb_info_buffer + nsm_offset);
    unsigned long count = 0;
    // Count the number of entries in the has table
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

    printf("\tsize: %i\n", table_size);
    printf("\tcapacity: %i\n", table_capacity);
    // TODO: I'm not sure how to actually read in the present and deleted bit vectors
    // they should tell us which indexes in the hash table can be read. But instead we're just going
    // to assume that they are in sequential order (which will probably bite us back later
    printf("\tpresent: %I64u\n", present_bit_vector);
    printf("\tdeleted: %I64u\n", deleted_bit_vector);

    printf("\tvalues:\n");
    for (int i = 0; i < table_size; i++) {
        unsigned int key = *(unsigned int*)(pdb_info_buffer + table_items_offset + i * 8);
        unsigned int value = *(unsigned int*)(pdb_info_buffer + table_items_offset + i * 8 + 4);
        if (key < BUFFER_SIZE) {
            printf("\t\tkey: %i\tvalue: %i\tstring:%s\n", key, value, pdb_info_buffer + nsm_offset + 4 + key);
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
        else {
            printf("\t\tkey: %i\tvalue: %i\n", key, value);
        }
    }

    // Read in the /names string table
    if (names_stream_index != 0) {

		// Read in the PDBInfo stream from index 1
		bytes_copied = 0;
		names_buffer = (char*)OSLayer::allocate_memory(stream_block_info[names_stream_index].num_bytes);
		for (int i = 0; i < stream_block_info[names_stream_index].num_blocks; i++) {
			block = *(unsigned int*)(buffer+stream_block_info[names_stream_index].byte_offset_to_blocks + (i*4));
			SetFilePointer(file_handle, block*pdb_header.page_size, NULL, FILE_BEGIN);
			success = ReadFile(file_handle, (char*)names_buffer+bytes_copied, pdb_header.page_size, &bytes_read, NULL);
			bytes_copied += bytes_read;
		}
    }



    // Parse the DBI Stream block;
    DbiStreamHeader dbi_stream_header = *(DbiStreamHeader*)(stream_buffer);
    printf("\n");
    printf("VersionSignature: %i\n", dbi_stream_header.version_signature);
    printf("VersionHeader: %i\n", dbi_stream_header.version_header);
    printf("Age: %i\n", dbi_stream_header.age);
    printf("Int size: %i\n", sizeof(dbi_stream_header.version_signature));
    printf("Global Stream Index: %d\n", dbi_stream_header.global_stream_index);
    printf("BuildNumber: %d\n", dbi_stream_header.build_number);
    printf("\n");


    unsigned int mod_info_offset = sizeof(DbiStreamHeader);
    unsigned long long num_source_files = 0;
    printf("Parsing ModInfo\n");
    char* module_buffer = (char*)OSLayer::allocate_memory(20000);
    unsigned long max_size = 20000;

    unsigned int file_info_offset = dbi_stream_header.mod_info_size 
        + dbi_stream_header.section_contribution_size 
        + dbi_stream_header.section_map_size
        + sizeof(dbi_stream_header);
    unsigned short num_modules = *(unsigned short*)(stream_buffer + file_info_offset);
    unsigned int file_names_offset = file_info_offset + 4 + 2*(num_modules*2);
    unsigned int num_source_files_2 = 0;
    unsigned int source_count_offset = file_info_offset + 4 + (num_modules*2);
    //unsigned int file_names_offset = file_info_offset + 4 + 2*(num_modules*2);
    //unsigned int names_buffer_offset = file_names_offset+num_source_files*4;
    for (int i = 0; i < num_modules; i++) {
        num_source_files_2 += *(unsigned short*)(stream_buffer + source_count_offset + (i * 2));
    }
    unsigned int names_buffer_offset = file_names_offset+num_source_files_2*4;
    unsigned int name_count = 0;
    int j = 0;
    while (stream_buffer[names_buffer_offset + j] != 0) {
        printf("%i: %i: \t %s\n", name_count, j, stream_buffer + names_buffer_offset+j);
        while (stream_buffer[names_buffer_offset + j] != '\0') {
            j++;
        }
        j++;
        name_count++;
    }




    while (mod_info_offset < sizeof(DbiStreamHeader) + dbi_stream_header.mod_info_size) {
		ModInfo mod_info = *(ModInfo*)(stream_buffer+mod_info_offset);
        if (mod_info.module_sym_stream != 0xFFFF) {
            StreamBlockInfo blocks_info = stream_block_info[mod_info.module_sym_stream];

            // Make sure we have enough memory to store all the blocks
            if (blocks_info.num_blocks * pdb_header.page_size > max_size) {
                OSLayer::free_memory((void*)module_buffer);
                max_size = blocks_info.num_blocks * pdb_header.page_size;
                module_buffer = (char*)OSLayer::allocate_memory(max_size);
            }

            // Clear the blocks
            for (int i = 0; i < max_size; i++) {
                module_buffer[i] = 0xcc;
            }

            // Copy the blocks to the module_buffer
            for (int i = 0; i < blocks_info.num_blocks; i++) {
                unsigned int block = *(unsigned int*)(buffer + blocks_info.byte_offset_to_blocks + (i * 4));
                SetFilePointer(file_handle, block * pdb_header.page_size, NULL, FILE_BEGIN);
                success = ReadFile(file_handle, (char*)module_buffer + i * pdb_header.page_size, pdb_header.page_size, &bytes_read, NULL);
                assert(success);
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
                        printf("Line numbers have column data\n");
                    }
					unsigned int range = *(unsigned int*)(module_buffer + line_information_offset + 16);
					unsigned int offset_file = *(unsigned int*)(module_buffer + line_information_offset + 20);
                    name_offset = *(unsigned int*)(module_buffer + checksum_offset + offset_file + 8);
					unsigned int num_pairs = *(unsigned int*)(module_buffer + line_information_offset + 24);
					unsigned int bytes_of_code = *(unsigned int*)(module_buffer + line_information_offset + 28);
                    //printf("Offset into names buffer: %i, module name: %s\n", name_offset, stream_buffer+mod_info_offset+sizeof(ModInfo));
                    printf("File name: %s\n", names_buffer + name_offset + 12);
                    for (int i = 0; i < num_pairs; i++) {
                        CVLine line_info = *(CVLine*)(module_buffer + line_information_offset + 32 + 8 * i);
						//printf("%i, %#08x\n", line_info.linenum_start, base_address+line_info.offset);
                    }
                }
				line_information_offset += size+8;
            }

        }

		mod_info_offset += sizeof(ModInfo);
		printf("\t\t\t\t\tIndex: %i\n", mod_info.SectionContribEntry.module_index);
		// It's possible that the next 16 bits are 0, and the Module Name starts right after that...
		if (*(unsigned short*)(stream_buffer + mod_info_offset) == 0) {
			mod_info_offset += 2;
		}
		num_source_files += mod_info.source_file_count;
		// Find next string
		printf("Module Name: %s\n", stream_buffer + mod_info_offset);
		if (stream_buffer[mod_info_offset] == 0) {
			mod_info_offset++;
		}
		while (stream_buffer[mod_info_offset] != 0) {
			mod_info_offset++;
		}
		mod_info_offset++;
		printf("ObjFileName: %s\n", stream_buffer + mod_info_offset);
		if (stream_buffer[mod_info_offset] == 0) {
			mod_info_offset++;
		}
		// Go to the next ModInfo offset 
		while (stream_buffer[mod_info_offset] != 0) {
			mod_info_offset++;
		}
		mod_info_offset++;
		// Align forward to a 4 byte alignment
		mod_info_offset = (mod_info_offset + 4 - 1) & ~(4 - 1);
		printf("\n\n");
    }

    printf("\n");
    printf("Parse File Info Substream:\n");
    /*
    unsigned int file_info_offset = dbi_stream_header.mod_info_size 
        + dbi_stream_header.section_contribution_size 
        + dbi_stream_header.section_map_size
        + sizeof(dbi_stream_header);
    unsigned short num_modules = *(unsigned short*)(stream_buffer + file_info_offset);
    */
    printf("Number of modules: %i\n", num_modules);
    printf("Number of source files: %i\n", num_source_files);
    printf("Module Indicies = {");
	for (int i = 0; i < num_modules; i++) {
        printf("%i, ", *(unsigned short*)(stream_buffer + file_info_offset + 4 + (i * 2)));
    }
	printf("}\n");
    //unsigned int num_source_files_2 = 0;
    num_source_files_2 = 0;
    //unsigned int source_count_offset = file_info_offset + 4 + (num_modules*2);
    //unsigned int file_names_offset = file_info_offset + 4 + 2*(num_modules*2);
    //unsigned int names_buffer_offset = file_names_offset+num_source_files*4;
    for (int i = 0; i < num_modules; i++) {
        unsigned short this_mod_src_count = *(unsigned short*)(stream_buffer + source_count_offset + (i * 2));
        num_source_files_2 += this_mod_src_count;
        printf("Mod %i has file count %i\n", i, this_mod_src_count);
    }
    printf("New source file count: %i\n", num_source_files_2);
    for (int i = 0; i < num_source_files; i++) { 
        unsigned int file_name_offset = *(unsigned int*)(stream_buffer + file_names_offset + (i * 4));
        printf("Source file name: %i, %i, %s\n", file_name_offset, i, stream_buffer+names_buffer_offset+file_name_offset);
    }
    printf("\n");

}

// Main code
int main(int, char**)
{
    char exe_path[] = "D:\\Projects\\CS5001_5002_Debugger\\src\\test_imgui\\x64\\Debug\\EasyDebugger.exe";
    char output_path[DEBUGGER_MAX_PATH] = {0};
    get_pdb_file_path_test(exe_path, output_path);

    printf("\n\nPDB path: %s\n", output_path);

    //parse_pdb_file(output_path);
    parse_pdb_file_2(output_path);

    return 0;


    ImGui_ImplWin32_EnableDpiAwareness();

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("EasyDebugger"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    
    Persistant_Vars vars;
    init_vars(&vars);

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        main_ui_loop(&vars);

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(vars.clear_color.x*255.0f), (int)(vars.clear_color.y*255.0f), (int)(vars.clear_color.z*255.0f), (int)(vars.clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
