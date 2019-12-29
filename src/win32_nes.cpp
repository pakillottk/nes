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
// #include "../vendor/imgui/imgui_demo.cpp"
#include "../vendor/imgui/imgui_impl_win32.cpp"
#include "../vendor/imgui/imgui_impl_opengl3.cpp"
#include "imgui_menu.cpp"

#define internal static
#define local_persist static
#define global_variable static

#define TO_RGB(R, G, B)  u32(0xff000000) | (R&0xff) << 16 | (G&0xFF) << 8  | (B&0xFF)

struct NESAppState
{
    HGLRC oglContext;
    NesCode nesCode;
    NESContext nesCtx;
    char currentROM[MAX_PATH];
};

#define ABS(V) (V) < 0 ? -V : V

NES_INIT(nesInitStub)
{}
NES_UPDATE(nesUpdateStub)
{}
NES_SHUTDOWN(nesShutdownStub)
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
        nesCode.shutdown = (nes_update*)GetProcAddress(nesCode.dllHandle, "NES_Shutdown");        
        nesCode.isValid = nesCode.initialize != NULL && nesCode.update != NULL && nesCode.shutdown != NULL;
    }

    if( !nesCode.isValid )
    {
        nesCode.initialize  = nesInitStub;
        nesCode.update      = nesUpdateStub;
        nesCode.shutdown    = nesShutdownStub;
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
        nesCode->shutdown = nesShutdownStub;     
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
        GL_RGBA, 
        NES_FRAMEBUFFER_WIDTH,
        NES_FRAMEBUFFER_HEIGHT,
        0,
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        nesContext->backbuffer
    );
}

internal bool8
Win32_MakeOpenGlContext(HWND Window, HGLRC *oglContext, NESContext *nesContext)
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

    *oglContext = wglCreateContext(dc);
    if( !wglMakeCurrent(dc, *oglContext) )
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
Win32_MakePatternTableTexture(NESContext *nesContext, NESCartridge *cartridge, byte i)
{
    u32 patternData[16 * 8 * 16 * 8 ];

    for( u16 tileY = 0; tileY < 16; ++tileY )
    {
        for( u16 tileX = 0; tileX < 16; ++tileX )
        {
			u16 offset = tileY * 256 + tileX * 16;
            for(i16 row = 0; row < 8; ++row)
			{
				byte tile_lsb = cartridge->VROM[i * 0x1000 + offset + row + 0x0000];
				byte tile_msb = cartridge->VROM[i * 0x1000 + offset + row + 0x0008];

				for (i16 col = 0; col < 8; ++col)
				{
					byte pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);
					tile_lsb >>= 1; tile_msb >>= 1;

                    byte x = tileX * 8 + (7 - col);
                    byte y = (tileY * 8 + row);

                    patternData[ (y * 16 * 8) + x ] = 0xffffff * (real32(pixel) / 4.0f);
				}
			}
        }
    }

    // Fill the texture
    glGenTextures(1, &nesContext->patternTableTexId[i]);
    glBindTexture(GL_TEXTURE_2D, nesContext->patternTableTexId[i]);
    glTexImage2D(   
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA8, 
        16 * 8,
        16 * 8,
        0,
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        patternData
    );

    // Config the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

internal void
Win32_MakeNameTableTexture(NESContext *nesContext, byte i)
{
    u32 nameTableData[ 32 * 32 ];

    for( u32 name = 0; name < 32 * 32; ++name )
    {
        byte tableName = nesContext->nes.ppu.nameTable[i][name];
        nameTableData[name] = TO_RGB( tableName, tableName, tableName );
    }

    // Fill the texture
    if( nesContext->nameTableTextId[i] == 0 )
    {
        glGenTextures(1, &nesContext->nameTableTextId[i]);
    }
    glBindTexture(GL_TEXTURE_2D, nesContext->nameTableTextId[i]);
    glTexImage2D(   
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA8, 
        32,
        32,
        0,
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        nameTableData
    );

    // Config the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

internal void
Win32_RenderOGL(NESContext *nesContext)
{
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    SwapNesBackbuffer(nesContext);

    glPushMatrix();
    
    glScalef(1.0f, -1.0f, 1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
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

    glPopMatrix();
}

internal bool8
Win32_RenderImGui(NesCode *nesCode, NESContext *nesContext, char *currentROM)
{
    bool8 quit = false;

    RenderState(nesContext);
    MENU_REQUEST request = RenderMainMenu(nesContext);    
    ImGui::Render();

    switch (request)
    {
        case kLoadRom:
        {            
            char path[MAX_PATH];
            if( OpenFileDialog(path) )
            {
                nesCode->initialize(nesContext, path);
                strcpy(currentROM, path);
                Win32_MakePatternTableTexture(nesContext, &nesContext->nes.cartridge, 0);
                Win32_MakePatternTableTexture(nesContext, &nesContext->nes.cartridge, 1);
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
            CREATESTRUCT *CreateStruct = (CREATESTRUCT *)LParam;
            NESAppState *appState = (NESAppState*)CreateStruct->lpCreateParams;
            SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)appState);
            if( !Win32_MakeOpenGlContext(Window, &appState->oglContext, &appState->nesCtx)    
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
        {
            NESAppState *appState = (NESAppState*)GetWindowLongPtr(Window, GWLP_USERDATA);
            wglDeleteContext(appState->oglContext);
            ImGui_ImplOpenGL3_Shutdown();
            ImGui::DestroyContext();
            ImGui_ImplWin32_Shutdown();

            appState->nesCode.shutdown(&appState->nesCtx);

            PostQuitMessage(0);
        }
        break;

        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            NESAppState *appState = (NESAppState*)GetWindowLongPtr(Window, GWLP_USERDATA);
            bool8 pressed = Message == WM_KEYDOWN;
            switch(WParam)
            {
                case 'Z':
                    appState->nesCtx.nes.gamepad[0].buttons.a = pressed;
                break;
                case 'X':
                    appState->nesCtx.nes.gamepad[0].buttons.b = pressed;
                break;
                case 'A':
                    appState->nesCtx.nes.gamepad[0].buttons.select = pressed;
                break;
                case 'S':
                    appState->nesCtx.nes.gamepad[0].buttons.start = pressed;
                break;
                case VK_LEFT:
                    appState->nesCtx.nes.gamepad[0].buttons.left = pressed;
                break;
                case VK_UP:
                    appState->nesCtx.nes.gamepad[0].buttons.up = pressed;
                break;
                case VK_RIGHT:
                    appState->nesCtx.nes.gamepad[0].buttons.right = pressed;
                break;
                case VK_DOWN:
                    appState->nesCtx.nes.gamepad[0].buttons.down = pressed;
                break;
            }
        }            
        break;

        case WM_PAINT:
        {
            NESAppState *appState = (NESAppState*)GetWindowLongPtr(Window, GWLP_USERDATA);
            HDC dc = GetDC(Window);
            wglMakeCurrent(dc, appState->oglContext);

            appState->nesCode.update(&appState->nesCtx);
            // u32 color = 0xff00ff00;
            // for( u32 i = 0; i < NES_FRAMEBUFFER_WIDTH * NES_FRAMEBUFFER_HEIGHT; ++i )
            // {
            //     // if( i > 0 && i % NES_FRAMEBUFFER_WIDTH == 0 )
            //     // {
            //     //     color >>= 16;
            //     //     if( color == 0x0 )
            //     //     {
            //     //         color = 0xff000000;
            //     //     }
            //     // }
            //     gNesCtx.backbuffer[ i ] = color;
            // }

            // render the nes framebuffer
            Win32_RenderOGL( &appState->nesCtx );    

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();            
            ImGui::NewFrame();    

            // render the NES State
            if( Win32_RenderImGui( &appState->nesCode, &appState->nesCtx, appState->currentROM ) )
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
    NESAppState appState;
    appState.nesCtx = {};    
    // TODO(pgm) For now starts with debugger enabled
    // appState.nesCtx.showDebugger = true;
    // gNesCtx.showPatternTables = true;

    appState.currentROM[0] = 0;

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
            &appState
        );

        appState.nesCode = LoadNesCode("NES.dll");

        for(;;)
        {
            #if HOT_RELOAD
                if( AttemptHotReload("NES.dll", &appState.nesCode) && appState.currentROM[0] != NULL )
                {
                    appState.nesCode.initialize(&appState.nesCtx, appState.currentROM);
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