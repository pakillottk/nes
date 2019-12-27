#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>

#include "nes_types.h"

// imgui source code
#include "../vendor/imgui/imgui.cpp"
#include "../vendor/imgui/imgui_draw.cpp"
#include "../vendor/imgui/imgui_widgets.cpp"
#include "../vendor/imgui/imgui_demo.cpp"
#include "../vendor/imgui/imgui_impl_win32.cpp"
#include "../vendor/imgui/imgui_impl_opengl3.cpp"

#define internal static
#define local_persist static

local_persist HGLRC gOglContext;

#define ABS(V) (V) < 0 ? -V : V

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

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();    
            ImGui::Render();

            Win32_RenderOGL();
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
        
        for(;;)
        {
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