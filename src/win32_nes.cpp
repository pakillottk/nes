#include <windows.h>
#include <gl/glew.h>
#include <gl/GL.h>

#include "nes_types.h"
#include "nes.h"
#include "win32_nes.h"

// imgui source code
#include "../vendor/imgui/imgui.cpp"
#include "../vendor/imgui/imgui_draw.cpp"
#include "../vendor/imgui/imgui_widgets.cpp"
#include "../vendor/imgui/imgui_demo.cpp"
#include "../vendor/imgui/imgui_impl_win32.cpp"
#include "../vendor/imgui/imgui_impl_opengl3.cpp"
#include "imgui_menu.cpp"

#define internal static
#define local_persist static
#define global_variable static

global_variable HGLRC gOglContext;
global_variable NesCode gNesCode;
global_variable NESContext gNesCtx;
global_variable char gCurrentROM[MAX_PATH];

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
OpenFileDialog(char *path)
{
    OPENFILENAME ofn;
    ZeroMemory( path, MAX_PATH );    
    ZeroMemory( &ofn, sizeof( OPENFILENAME ) );

    ofn.lStructSize=sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "NES ROM\0*.nes";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrFileTitle = "Load NES ROM";
    ofn.lpstrDefExt = "nes";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    return GetOpenFileNameA( &ofn );
}

internal void 
SwapNesBackbuffer(NESContext *nesContext)
{
    // Fill the texture
    glBindTexture(GL_TEXTURE_2D, nesContext->frameBufferTextId);
    glTexImage2D(   
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA8, 
        NES_FRAMEBUFFER_WIDTH,
        NES_FRAMEBUFFER_HEIGHT,
        0,
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        nesContext->backbuffer
    );
}

internal bool8
Win32_MakeOpenGlContext(HWND Window, NESContext *nesContext)
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

    // we want to render in screen space
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glGenTextures(1, &nesContext->frameBufferTextId);
    
    // generate a pitch black backbuffer
    ZeroMemory( nesContext->backbuffer, sizeof(nesContext->backbuffer) );    
    SwapNesBackbuffer(nesContext); 

    // Config the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

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
Win32_RenderOGL(NESContext *nesContext)
{
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    SwapNesBackbuffer(nesContext);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 1.0f, 1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
    glEnd();
}

internal bool8
Win32_RenderImGui(NesCode *nesCode, NESContext *nesContext)
{
    bool8 quit = false;

    RenderState(nesContext);
    MENU_REQUEST request = RenderMainMenu();    
    ImGui::Render();

    switch (request)
    {
        case kLoadRom:
        {            
            char path[MAX_PATH];
            if( OpenFileDialog(path) )
            {
                nesCode->initialize(nesContext, path);
                strcpy(gCurrentROM, path);
            }
        }            
        break;

        case kQuit:
            quit = true;
        break;

        default:
            break;
    }

    return(quit);
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
            if( !Win32_MakeOpenGlContext(Window, &gNesCtx)    
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
            Win32_RenderOGL( &gNesCtx );    

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();            
            ImGui::NewFrame();    

            // render the NES State
            if( Win32_RenderImGui( &gNesCode, &gNesCtx ) )
            {
                PostQuitMessage(0);
            }
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
    gCurrentROM[0] = 0;

    WNDCLASS wc = {};    
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = (HICON)LoadImageA(     // returns a HANDLE so we have to cast to HICON
                    NULL,             // hInstance must be NULL when loading from a file
                    "data/nes.ico",   // the icon file name
                    IMAGE_ICON,       // specifies that the file is an icon
                    0,                // width of the image (we'll specify default later on)
                    0,                // height of the image
                    LR_LOADFROMFILE|  // we want to load a file (as opposed to a resource)
                    LR_DEFAULTSIZE|   // default metrics based on the type (IMAGE_ICON, 32x32)
                    LR_SHARED         // let the system release the handle when it's no longer used
                );
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

        for(;;)
        {
            #if HOT_RELOAD
                if( AttemptHotReload("NES.dll", &gNesCode) && gCurrentROM[0] != NULL )
                {
                    gNesCode.initialize(&gNesCtx, gCurrentROM);
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