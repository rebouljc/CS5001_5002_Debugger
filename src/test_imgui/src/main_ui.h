#pragma once

#include "Persistant_Vars.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
//#define DIRECTINPUT_VERSION 0x0602 
//#include <dinput.h>
#include <tchar.h>

#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
void init_vars(struct Persistant_Vars *vars);
void main_ui_loop(struct Persistant_Vars *vars);
// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);