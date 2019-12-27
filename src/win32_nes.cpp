#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>

#include "nes_types.h"
#include "nes.h"

// imgui source code
#include "../vendor/imgui/imgui.cpp"
#include "../vendor/imgui/imgui_draw.cpp"
#include "../vendor/imgui/imgui_widgets.cpp"
#include "../vendor/imgui/imgui_demo.cpp"
#include "../vendor/imgui/imgui_impl_win32.cpp"
#include "../vendor/imgui/imgui_impl_opengl3.cpp"

#define internal static
#define local_persist static
#define global_variable static

struct NesCode
{
    bool8 isValid;
    HMODULE dllHandle;
    FILETIME lastWriteTime;
    nes_init *initialize;
    nes_update *update;    
};

global_variable HGLRC gOglContext;
global_variable NesCode gNesCode;
global_variable NESContext gNesCtx;

#define ABS(V) (V) < 0 ? -V : V

NES_INIT(nesInitStub)
{}
NES_UPDATE(nesUpdateStub)
{}

internal FILETIME Win32_GetLastWriteTime(const char *filename)
{
    FILETIME lastWriteTime = {};

    WIN32_FIND_DATA findData;
    HANDLE fHandle = FindFirstFileA(filename, &findData);
    if( fHandle != INVALID_HANDLE_VALUE )
    {
        lastWriteTime = findData.ftLastWriteTime;
        FindClose(fHandle);
    }
    return(lastWriteTime);
}

// used for hot reload
global_variable const char *liveDll = "NESLive.dll";

internal 
NesCode LoadNesCode(const char *targetFilename)
{
    NesCode nesCode;
    nesCode.isValid = 0;
    nesCode.lastWriteTime = Win32_GetLastWriteTime(targetFilename);

    #if HOT_RELOAD
        CopyFileA(targetFilename, liveDll, false);
        nesCode.dllHandle = LoadLibraryA(liveDll);
    #else
        nesCode.dllHandle = LoadLibraryA(targetFilename);
    #endif

    if( nesCode.dllHandle )
    {
        nesCode.initialize = (nes_init*)GetProcAddress(nesCode.dllHandle, "NES_Init");
        nesCode.update = (nes_update*)GetProcAddress(nesCode.dllHandle, "NES_Update");        
        nesCode.isValid = nesCode.initialize != NULL && nesCode.update != NULL;
    }

    if( !nesCode.isValid )
    {
        nesCode.initialize = nesInitStub;
        nesCode.update  = nesUpdateStub;        
    }

    return nesCode;
} 

internal void 
UnloadNesCode(NesCode *nesCode)
{
    if( nesCode->isValid )
    {
        nesCode->isValid = 0;
        nesCode->initialize = nesInitStub;
        nesCode->update  = nesUpdateStub;        
        FreeLibrary( nesCode->dllHandle );
    }
}

#if HOT_RELOAD
internal bool8 
AttemptHotReload(const char * targetFilename, NesCode *nesCode)
{
    FILETIME targetWriteTime = Win32_GetLastWriteTime(targetFilename);
    if( CompareFileTime(&targetWriteTime, &nesCode->lastWriteTime) != 0 )
    {
        // reload
        UnloadNesCode(nesCode);
        *nesCode = LoadNesCode(targetFilename);

        return(TRUE);
    }

    return(FALSE);
}
#endif

internal bool8
Win32_MakeOpenGlContext(HWND Window)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    HDC dc = GetDC(Window);

    int  pixelFormat;
    pixelFormat = ChoosePixelFormat(dc, &pfd); 
    SetPixelFormat(dc, pixelFormat, &pfd);

    gOglContext = wglCreateContext(dc);
    if( !wglMakeCurrent(dc, gOglContext) )
    {
        // TODO(pgm) Proper error handling
        MessageBoxA(Window, "The opengl context failed...", "Fatal error", 0);
        return(false);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        // TODO(pgm) Proper error handling
        MessageBoxA(Window, "The opengl context failed...", "Fatal error", 0);
        return(false);
    }

    return(true);
}

internal bool8
Win32_InitializeImGui(HWND Window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;  
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls    

    if( !ImGui_ImplWin32_Init(Window) )
    {
        return(false);
    }
    if( !ImGui_ImplOpenGL3_Init() )
    {
        return(false);
    }

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();   

    return(true);
}

internal void
Win32_RenderOGL()
{
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT);
}

internal void
Win32_RenderImGui()
{
    ImGui::ShowDemoWindow();
}

internal LRESULT CALLBACK 
WindowProc(HWND   Window,
           UINT   Message,
           WPARAM WParam,
           LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CREATE:
        {
            if( !Win32_MakeOpenGlContext(Window)    
                || !Win32_InitializeImGui(Window))
            {
                // exit the app if this fails...
                PostQuitMessage(0);
            }
        }
        break;

        case WM_SIZE:         
        {
            u32 w, h;
            RECT wndRect;            
            GetWindowRect(Window, &wndRect);
            w = ABS(wndRect.right - wndRect.left);
            h = ABS(wndRect.bottom - wndRect.top);

            glViewport(0, 0, w, h);

            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize.x = w;
            io.DisplaySize.y = h;
        }               
        break;

        case WM_DESTROY:
            wglDeleteContext(gOglContext);
            ImGui_ImplOpenGL3_Shutdown();
            ImGui::DestroyContext();
            ImGui_ImplWin32_Shutdown();

            PostQuitMessage(0);
        break;

        case WM_PAINT:
        {
            HDC dc = GetDC(Window);
            wglMakeCurrent(dc, gOglContext);

            gNesCode.update(&gNesCtx);

            // render the nes framebuffer
            Win32_RenderOGL();    

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();            
            ImGui::NewFrame();    

            // render the NES State
            Win32_RenderImGui();

            ImGui::Render();    
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SwapBuffers(dc);
        }
        break;
        
        default:
            ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam);
            Result = DefWindowProc(Window, Message, WParam, LParam);
        break;
    }

    return(Result);
}

int CALLBACK 
WinMain(HINSTANCE hInstance, 
        HINSTANCE hPrevInstance,
        LPSTR     lpCmdLine,
        int       nShowCmd)
{
    gNesCtx = {};

    WNDCLASS wc = {};    
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("NESWNDClass");
    
    if( RegisterClass(&wc) )
    {
        HWND wnd = CreateWindowEx(
            0,
            wc.lpszClassName,
            "NES",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            hInstance,
            NULL
        );

        gNesCode = LoadNesCode("NES.dll");
        gNesCode.initialize(&gNesCtx);

        for(;;)
        {
            #if HOT_RELOAD
                if( AttemptHotReload("NES.dll", &gNesCode) )
                {
                    gNesCode.initialize(&gNesCtx);
                }
            #endif

            MSG Message;
            BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
            if( MessageResult > 0 )
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        // Alert of failure
        MessageBoxA(NULL, "Couldn't register the window class...", "Fatal error", 0);
    }
    

    return(0);
}