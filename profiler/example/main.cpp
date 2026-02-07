// Tracy Profiler Embedded Example
// Self-profiling example using GLFW + OpenGL3 + ImGui

#include <stdio.h>
#include <memory>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_opengl3_loader.h>
#include <GLFW/glfw3.h>

// Tracy client for self-profiling
#include <tracy/Tracy.hpp>

// Tracy profiler UI
#include "profiler/TracyView.hpp"
#include "profiler/TracyTexture.hpp"
#include "profiler/TracyConfig.hpp"
#include "Fonts.hpp"

static std::shared_ptr<tracy::View> s_view;

static void RunOnMainThread(const std::function<void()>& cb, bool = false) { cb(); }
static void SetWindowTitleCallback(const char*) {}
static void SetupScaleCallback(float) {}
static void AttentionCallback() {}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Tracy Embedded Example", nullptr, nullptr);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    tracy::InitTexture();
    tracy::LoadConfig();
    LoadFonts(1.0f);

    // Connect to ourselves (embedded mode)
    s_view = std::make_shared<tracy::View>(
        RunOnMainThread, "127.0.0.1", 8086,
        SetWindowTitleCallback, SetupScaleCallback, AttentionCallback, nullptr,
        true  // embedded
    );

    while (!glfwWindowShouldClose(window))
    {
        ZoneScoped;

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a fullscreen dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();

        // Tracy profiler - we create the window, Tracy fills it
        if (s_view)
        {
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoScrollbar);
            s_view->Draw();
            ImGui::End();
        }

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        FrameMark;
    }

    s_view.reset();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
