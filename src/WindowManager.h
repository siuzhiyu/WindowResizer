#pragma once

#include <windows.h>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RenderLoop();
void InitAppData();
void FindWindowCallback();
void HandleWindowSelection(HWND hwndHit);
void StartMouseHook(HWND hwnd);
void StopMouseHook();
void SetMainViewportWindow(HWND hwnd);
