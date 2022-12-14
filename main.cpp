// C++
#include <vector>
#include <string>
#include <iostream>
// SDL
#include <glad/glad.h>
#include <SDL.h>
// ImGUI
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

int windowWidth = 1280,
    windowHeight = 720;

static bool show_controls = true;
static bool show_settings = false;

void showControls(bool *p_open, SDL_Window *window, float menubar_height);
void showViewer(SDL_Window *window, float &menubar_height);
void showSettings(bool *p_open, SDL_Window *window);

int main()
{

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "[ERROR] " << SDL_GetError() << std::endl;
        return -1;
    }

    // set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE
    );

    std::string glsl_version;
#ifdef __APPLE__
    // GL 3.2 Core + GLSL 150
    glsl_version = "#version 150";
    SDL_GL_SetAttribute( // required on Mac OS
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
    // GL 3.2 Core + GLSL 150
    glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
            SDL_WINDOW_OPENGL
            | SDL_WINDOW_RESIZABLE
            | SDL_WINDOW_ALLOW_HIGHDPI
    );
    SDL_Window *window = SDL_CreateWindow(
            "slimulation",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            windowWidth,
            windowHeight,
            window_flags
    );
    // limit to which minimum size user can resize the window
    SDL_SetWindowMinimumSize(window, 500, 300);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    // enable VSync
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
    }
    else
    {
        std::cout << "[INFO] glad initialized\n";
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // setup Dear ImGui style
    ImGui::StyleColorsDark();

    // setup platform/renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    // set default font
    // this uses .. as the font directory, may have to change for releases
    io.Fonts->AddFontFromFileTTF("../NimbusSans-Regular.otf", 18);

    // to update font size/style in runtime:
    // io.Fonts->Clear();
    // io.Fonts->AddFontFromFileTTF("font_file", font_size);

    // colors are set in RGBA, but as float
    ImVec4 background = ImVec4(35/255.0f, 35/255.0f, 35/255.0f, 1.00f);

    glClearColor(background.x, background.y, background.z, background.w);
    bool loop = true;
    while (loop)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // without it you won't have keyboard input and other things
            ImGui_ImplSDL2_ProcessEvent(&event);
            // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
            // before processing events

            switch (event.type)
            {
                case SDL_QUIT:
                    loop = false;
                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                            windowWidth = event.window.data1;
                            windowHeight = event.window.data2;
                            // std::cout << "[INFO] Window size: "
                            //           << windowWidth
                            //           << "x"
                            //           << windowHeight
                            //           << std::endl;
                            glViewport(0, 0, windowWidth, windowHeight);
                            break;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            loop = false;
                            break;
                    }
                    break;
            }
        }

        // start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        float menubar_height{ 0 };

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Test")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Settings", "")) { show_settings = true; }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Controls", "", show_controls)) { show_controls = !show_controls; }
                ImGui::EndMenu();
            }
            menubar_height = ImGui::GetWindowSize().y;
            ImGui::EndMainMenuBar();
        }

        showViewer(window, menubar_height);

        if (show_controls)
            showControls(&show_controls, window, menubar_height);

        if (show_settings)
            showSettings(&show_settings, window);

        // rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void showControls(bool *p_open, SDL_Window *window, float menubar_height)
{
    // get the window size as a base for calculating widgets geometry
    int sdl_width = 0, sdl_height = 0, controls_width = 0;
    SDL_GetWindowSize(window, &sdl_width, &sdl_height);
    controls_width = sdl_width;
    // make controls widget width to be 1/4 of the main window width
    controls_width /= 4;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(0, menubar_height), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // the height of the main window also with some margin
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(controls_width), static_cast<float>(sdl_height - menubar_height)),
            ImGuiCond_Always
    );
    // create a window and append into it
    ImGui::Begin("Controls", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
    ImGui::Text("%s", SDL_GetPlatform());
    ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
    ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);

    // buttons and most other widgets return true when clicked/edited/activated
    if (ImGui::Button("Start"))
    {
        std::cout << "Start button clicked" << std::endl;
    }

    ImGui::End();
}

void showViewer(SDL_Window *window, float &menubar_height)
{
    // get the window size as a base for calculating widgets geometry
    int sdl_width = 0, sdl_height = 0, viewer_width = 0;
    SDL_GetWindowSize(window, &sdl_width, &sdl_height);
    viewer_width = sdl_width;
    // make viewer widget width to be 3/4 of the main window width
    if (show_controls)
        viewer_width *= 0.75;

    // position the controls widget in the top-left corner with some margin
    ImGui::SetNextWindowPos(ImVec2(sdl_width - viewer_width, menubar_height), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // the height of the main window also with some margin
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(viewer_width), static_cast<float>(sdl_height - menubar_height)),
            ImGuiCond_Always
    );
    // create a window and append into it
    ImGui::Begin("Viewer", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

    ImGui::Text("Slime simulation through OpenGL goes here");

    ImGui::End();
}

void showSettings(bool *p_open, SDL_Window *window)
{
    ImGui::Begin("Settings", p_open);
    int n = 0;
    ImGui::Combo("Font Size", &n, "12\0 14\0\0");
    ImGui::End();
}
