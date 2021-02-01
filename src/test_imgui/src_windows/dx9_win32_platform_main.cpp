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

struct GUID {
    unsigned char signature[4];
    unsigned long guid1;
    unsigned short guid2;
    unsigned short guid3;
    unsigned char guid4[8];
};

struct PDBStream {
    unsigned int version;
    unsigned int signature;
    unsigned int age;
    union {
        GUID guid;
        struct {
            unsigned char signature[4];
            unsigned long guid1;
            unsigned short guid2;
            unsigned short guid3;
            unsigned char guid4[8];
        };
    };
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

    while (debug_dir_file_offset >= (current_start + size)) {
		success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
        if (!success) {
            printf("Reached end of file?\n");
            return;
        }
        current_start += bytes_read;
    }

    DEBUG_DIR another_debug_dir = *(DEBUG_DIR*)(buffer+(debug_dir_file_offset-current_start));
    
    if (another_debug_dir.type == 2) {
        printf("Debug data is Visual C++ debug information\n");
        printf("Major version: %i Minor Version %i\n", another_debug_dir.major_version, another_debug_dir.minor_version);
    }


    while (another_debug_dir.pointer_to_raw_data >= (current_start + size)) {
		success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
        if (!success) {
            printf("Reached end of file?\n");
            return;
        }
        current_start += bytes_read;
    }

    unsigned long code_view_offset = + another_debug_dir.pointer_to_raw_data - current_start;
    CODEVIEW codeview = *(CODEVIEW*)(buffer +code_view_offset);
    printf("\tcodeview header:\n");
    printf("\t\tSignature: ");
    for (int i = 0; i < 4; i++) {
        printf("%c", codeview.signature[i]);
    }
    printf("\n\t\tGUID: %#04X-%#02X-%#02X-%#08X\n", codeview.guid1, codeview.guid2, codeview.guid3, codeview.guid4);
    printf("\t\tage: %i\n",codeview.age);

    printf("\t\tFile Path: %s\n", (buffer+code_view_offset+sizeof(codeview)));

	char* pdb_file_path = (buffer + code_view_offset + sizeof(codeview));

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
            }
            printf("%i, ", block);
        }
        printf("}\n");
        total_blocks += stream_block_size;
    }

    printf("\ttotal counted blocks: %i\n", total_blocks);
    printf("\n");

    printf("\tNow reading the PDB stream\n");

	// Go to the pdb stream 
    SetFilePointer(file_handle, pdb_header_block*pdb_header.page_size, NULL, 0 );
    success = ReadFile(file_handle, buffer, BUFFER_SIZE, &bytes_read, NULL);

    PDBStream s = *(PDBStream*)(buffer);

    if (s.version == 20000404) {
        printf("\t\tGot VC70\n");
    }
    else {
        printf("\t\tGot a different version number!!!\n");
    }

    printf("\t\tGUID: %#04X-%#02X-%#02X-%#08X\n", s.guid1, s.guid2, s.guid3, s.guid4);




    CloseHandle(file_handle);
    printf("Closed pdb file\n");
}

// Main code
int main(int, char**)
{
    char exe_path[] = "D:\\Projects\\CS5001_5002_Debugger\\src\\test_imgui\\x64\\DebugCopy\\EasyDebugger.exe";
    char output_path[DEBUGGER_MAX_PATH] = {0};
    get_pdb_file_path_test(exe_path, output_path);

    printf("\n\nPDB path: %s\n", output_path);

    parse_pdb_file(output_path);

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
