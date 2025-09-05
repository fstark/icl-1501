#include "ui.hpp"
#include "utils.hpp"
#include "disassembler.hpp"
#include "assembler.hpp"
#include "binary.hpp"
#include "emulator.hpp"
#include "addrs.hpp"
#include "memory.hpp"
#include "crt.hpp"

#include <iostream>
#include <format>
#include <vector>

// ImGui and SDL includes - only in the .cpp file
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// UI state variables
bool show_data = true;
bool show_assembler = true;
addrs_t sAddrs("P00-000");
addrs_t sIaw("P00-000");

// Static buffer for multiline text editor
static char text_editor_buffer[1024 * 64] = "// Enter your assembly code here\n// Example:\n ORG P00-100\n LDX R#1 030\n IOC C#0 007\n STA I#1 P00\n";

// Static variables for assembly results
static std::string assembly_result;
static std::string assembly_errors;
static bool assembly_successful = false;
static bool loaded_into_memory = false;

// Constants for ImGui IDs
static const int kID_PC = 0;
static const int kID_SP = 1;
static const int kID_CRT = 2;
static const int kID_FONT = 3;
static const int kID_ALT_FONT = 4;
static const int kID_SP_BASE = 1000;

#define IAW_COLOR IM_COL32(140, 80, 80, 255) // Current instruction

static void render_addrs(addrs_t addrs)
{
    std::string label = addrs.as_string();

    static bool inited = false;
    static ImVec2 text_size;

    if (!inited)
    {
        // Get text size for background rectangle
        text_size = ImGui::CalcTextSize("P00-000");
        inited = true;
    }

    if (addrs==sIaw)
    {
        // Draw light blue background rectangle
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 padding(4.0f, 2.0f);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        draw_list->AddRectFilled(
            ImVec2(pos.x - padding.x, pos.y - padding.y),
            ImVec2(pos.x + text_size.x + padding.x, pos.y + text_size.y + padding.y),
            IAW_COLOR
        );
    }

    ImGui::Text("%s", label.c_str());
}

static void render_memory_cell(const memory_t &memory, const addrs_t &addrs)
{
    static bool inited = false;
    static ImVec2 text_size;

    if (!inited)
    {
        // Get text size for background rectangle
        text_size = ImGui::CalcTextSize("000");
        inited = true;
    }

    if (addrs == sIaw || addrs == sIaw+1)
    {
        // Draw light blue background rectangle
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 padding(4.0f, 2.0f);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        draw_list->AddRectFilled(
            ImVec2(pos.x - padding.x, pos.y - padding.y),
            ImVec2(pos.x + text_size.x + padding.x, pos.y + text_size.y + padding.y),
            IAW_COLOR);
    }

    std::string label = to_octal(memory.get(addrs));
    ImGui::Text("%s", label.c_str());
}

void render_addrs(const char *label, addrs_t addrs)
{
    ImGui::Text("%s", label);
    ImGui::SameLine();
    render_addrs(addrs);
}

void render_active_addrs(addrs_t addrs, int id)
{
    std::string label = addrs.as_string();
    ImGui::PushID(id);
    if (ImGui::Button(label.c_str()))
    {
        show_data = true;
        sAddrs = addrs;
    }
    ImGui::PopID();
}

void render_active_addrs(const char *label, addrs_t addrs, int id)
{
    ImGui::Text("%s", label);
    ImGui::SameLine();
    render_active_addrs(addrs,id);
}

void render_memory(const memory_t &memory, const addrs_t &addrs)
{
    ImGui::Begin("Memory", &show_data);

    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Explore"))
        {
            ImGui::Text("%s", std::format("Section: {}, Level: {}", addrs.section(), addrs.level()).c_str());
            addrs_t a = addrs;
            for (int l = 0; l != 16; l++)
            {
                render_addrs(a);
                ImGui::SameLine();
                ImGui::Text(": ");
                for (int c = 0; c != 16; c++)
                {
                    ImGui::SameLine();
                    render_memory_cell( memory, a );
                    a = a + 1;
                }
            }
            if (ImGui::Button("Prev"))
            {
                sAddrs = sAddrs + (-256);
            }
            ImGui::SameLine();
            if (ImGui::Button("Next"))
            {
                sAddrs = sAddrs + 256;
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Disassembly"))
        {
            disassembler_t disassembler;
            ImGui::Text("%s", std::format("Section: {}, Level: {}", addrs.section(), addrs.level()).c_str());
            addrs_t a = addrs;
            for (int l = 0; l != 16; l++)
            {
                if (a == sIaw)
                    ImGui::Text("->");
                else
                    ImGui::Text("  ");
                ImGui::SameLine();
                render_addrs(a);
                ImGui::SameLine();
                ImGui::Text(": ");
                iw_t w = memory.get_instruction(a);
                ImGui::SameLine();
                ImGui::Text("%s      %s", w.as_octal().c_str(), disassembler.disassemble(w).c_str());
                a = a.next_instruction();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void render_assembler(emulator_t& emu)
{
    ImGui::Begin("Assembler", &show_assembler);
    
    // Multiline text editor taking up about 60% of the window
    ImGui::Text("Assembly Code Editor:");
    ImGui::Separator();
    
    // Calculate heights for the split layout
    ImVec2 window_size = ImGui::GetWindowSize();
    float editor_height = window_size.y * 0.6f - 100; // Reserve space for buttons and padding
    float output_height = window_size.y * 0.35f;
    
    // Text editor section
    ImGui::BeginChild("TextEditor", ImVec2(0, editor_height), true);
    bool text_changed = ImGui::InputTextMultiline("##source", text_editor_buffer, sizeof(text_editor_buffer),
                             ImVec2(-FLT_MIN, -FLT_MIN),
                             ImGuiInputTextFlags_AllowTabInput);
    ImGui::EndChild();
    
    // Auto-assemble on text change
    if (text_changed) {
        // Clear previous errors but keep successful results
        assembly_errors.clear();
        loaded_into_memory = false; // Reset load status when text changes
        
        try {
            // Use the new clean assembly free function from assembler.cpp
            binary_t binary = assemble(text_editor_buffer);
            assembly_result = to_string(binary);
            assembly_successful = true;

            store(emu.memory(), binary);
            // emu.render_crt();
        }
        catch (const std::exception &e) {
            assembly_errors = e.what();
            assembly_successful = false;
            // Keep the previous assembly_result intact
        }
    }
    
    // Action buttons
    ImGui::Separator();
    if (ImGui::Button("Clear"))
    {
        text_editor_buffer[0] = '\0';
        // Clear results when clearing text
        assembly_result.clear();
        assembly_errors.clear();
        assembly_successful = false;
        loaded_into_memory = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Example"))
    {
        const char* example = "// Example ICL-1501 Assembly Code\n ORG P00-000\n LDX R#1 030        ; Load index register 1 with 030\n IOC C#0 007        ; I/O control - tape transfer\n STA I#1 P00        ; Store accumulator indirect\n CPX R#1 230        ; Compare index register 1 with 230\n BRL P1-002         ; Branch if less\n IOC C#0 016        ; I/O control - select deck\n IOC C#0 005        ; I/O control - stop tape\n BRU P0-030         ; Branch unconditional\n";
        strncpy(text_editor_buffer, example, sizeof(text_editor_buffer) - 1);
        text_editor_buffer[sizeof(text_editor_buffer) - 1] = '\0';
        loaded_into_memory = false; // Reset load status when loading example
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy Output"))
    {
        if (assembly_successful && !assembly_result.empty()) {
            ImGui::SetClipboardText(assembly_result.c_str());
        }
    }
    ImGui::SameLine();

    
    // Output section
    ImGui::Separator();
    ImGui::Text("Assembly Output:");
    
    // Reserve space for error display at bottom
    float error_section_height = 60.0f; // Fixed height for error messages
    float adjusted_output_height = output_height - error_section_height;
    
    ImGui::BeginChild("AssemblyOutput", ImVec2(0, adjusted_output_height), true);
    
    // Show assembly results if we have any - no error messages here
    if (!assembly_result.empty()) {
        if (assembly_successful) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Assembly successful!");
            if (loaded_into_memory) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0, 0.8f, 1, 1), " [Loaded into memory]");
            }
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Previous successful assembly:");
        }
        ImGui::Separator();
        ImGui::TextUnformatted(assembly_result.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "No assembly output yet. Type assembly code above to see results.");
    }
    
    ImGui::EndChild();
    
    // Separate error section at the bottom - fixed position
    ImGui::BeginChild("ErrorSection", ImVec2(0, error_section_height), true, ImGuiWindowFlags_NoScrollbar);
    if (!assembly_errors.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Assembly Error:");
        ImGui::TextWrapped("%s", assembly_errors.c_str());
    } else {
        ImGui::TextColored(ImVec4(0, 0.8f, 0, 1), "Ready");
    }
    ImGui::EndChild();
    
    ImGui::End();
}

void render_screen(const crt_t::screen_buffer_t &screen)
{
    static GLuint texture = 0;
    static int last_width = 0, last_height = 0;
    const int width = crt_t::matrix_width_ * crt_t::screen_columns_;
    const int height = crt_t::matrix_height_ * crt_t::screen_lines_;

    // Convert screen buffer to RGBA pixels
    std::vector<unsigned char> pixels(width * height * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            char c = screen[y][x];
            unsigned char v = (c == ' ' || c == 0) ? 0 : 255; // black for space, white otherwise
            int idx = (y * width + x) * 4;
            pixels[idx + 0] = v; // R
            pixels[idx + 1] = v; // G
            pixels[idx + 2] = v; // B
            pixels[idx + 3] = 255; // A
        }
    }

    // Create or update texture
    if (!texture || last_width != width || last_height != height) {
        if (texture) glDeleteTextures(1, &texture);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        last_width = width;
        last_height = height;
    } else {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }

    ImGui::Begin("Screen");
    ImGui::Image((void*)(intptr_t)texture, ImVec2(width * 2, height * 2)); // scale up for visibility
    ImGui::End();
}

void render_breakpoints( emulator_t &emu )
{
    auto &cpu = emu.cpu();
    ImGui::Begin("Breakpoints");
    static int new_bp = 0;
    ImGui::InputInt("New Breakpoint", &new_bp);
    ImGui::SameLine();
    if (ImGui::Button("Add"))
    {
        emu.breakpoints().add_breakpoint( addrs_t(new_bp), eBreakpointType::EXECUTE );
    }
    ImGui::Separator();
    ImGui::Text("Current Breakpoints:");
    for (auto &bp : emu.breakpoints().breakpoints())
    {
        bool enabled = bp.enabled();
        ImGui::Checkbox(std::format("##enable_{}", bp.addr().as_string()).c_str(), &enabled);
        bp.set_enabled(enabled);

        ImGui::SameLine();
        ImGui::Text("%s", bp.addr().as_string().c_str());
        ImGui::SameLine();
        bool active = bp.mask() & eBreakpointType::READ;
        ImGui::Checkbox(std::format("##read_{}", bp.addr().as_string()).c_str(), &active);
        bp.update_mask( eBreakpointType::READ, active );
        ImGui::SameLine();
        ImGui::Text("READ");
        ImGui::SameLine();
        active = bp.mask() & eBreakpointType::WRITE;
        ImGui::Checkbox(std::format("##write_{}", bp.addr().as_string()).c_str(), &active);
        bp.update_mask( eBreakpointType::WRITE, active );
        ImGui::SameLine();
        ImGui::Text("WRITE");
        ImGui::SameLine();
        active = bp.mask() & eBreakpointType::EXECUTE;
        ImGui::Checkbox(std::format("##execute_{}", bp.addr().as_string()).c_str(), &active);
        bp.update_mask( eBreakpointType::EXECUTE, active );
        ImGui::SameLine();
        ImGui::Text("EXECUTE");
        ImGui::SameLine();
        if (ImGui::Button(std::format("Remove##{}", bp.addr().as_string()).c_str()))
        {
            emu.breakpoints().remove_breakpoint(bp.addr());
            break; // Break to avoid iterator invalidation
        }
    }
    ImGui::End();
}

void render_internals(emulator_t &emu)
{
    auto &cpu = emu.cpu();

    ImGui::Begin("Control Panel");
    if (ImGui::Button("Reset"))
    {
        cpu.reset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step"))
    {
        cpu.step();
    }
    ImGui::SameLine();
    if (ImGui::Button("Assembler"))
    {
        show_assembler = true;
    }
    ImGui::End();

    ImGui::Begin("CPU");
        auto pc = cpu.iaw();
        auto iw = cpu.memory().get_instruction(pc);
        render_active_addrs("PC: ", pc, kID_PC);
        ImGui::SameLine();
        ImGui::Text("IW: %s", iw.as_octal().c_str());
        static disassembler_t disassembler;
        ImGui::Text("%s", disassembler.disassemble(iw).c_str());

        ImGui::Separator();
        ImGui::Text("   SP: %d", cpu.sp());
        ImGui::Text("Stack:");
        for (int i = 0; i < 8; ++i)
        {
            if (i == cpu.sp())
                ImGui::Text("*");
            else
                ImGui::Text(" ");
            ImGui::SameLine();
            render_active_addrs(cpu.sp_base(i), kID_SP + 1 + i * 2);
            ImGui::SameLine(80);
            render_active_addrs(cpu.memory().get_addrs(cpu.sp_base(i)), kID_SP + 1 + i * 2 + 1);
        }

        ImGui::Separator();
        static const char *compare_str[] = {"L", "E", "H"};
        ImGui::Text("ACC: %s", to_octal(cpu.io().accumulator()).c_str());
        ImGui::SameLine();
        ImGui::Text("Compare: %s", compare_str[cpu.compare_]);

        ImGui::Text("Index Registers:");
        for (int i = 1; i <= 8; ++i)
        {
            ImGui::BeginGroup();
            ImGui::Text("R#%d", i);
            ImGui::Text("%s", to_octal(cpu.index_register(i)).c_str());
            ImGui::EndGroup();
            if (i < 8)
                ImGui::SameLine();
        }
    ImGui::End();

    ImGui::Begin("CRT");
        auto crt = emu.io().crt();

        render_active_addrs("  Screen: ", crt.screen(), kID_CRT);
        render_active_addrs("    Font: ", crt.font(), kID_FONT);
        render_active_addrs("Alt Font: ", crt.alt_font(), kID_ALT_FONT);
    ImGui::End();

    ImGui::Begin("Tapes");
        auto io = emu.io();

        for (int i = 0; i != io.tape_count(); i++)
        {
            auto &tape_reader = io.tape_reader(i);
            if (i == io.tape_index())
                ImGui::Text("->");
            else
                ImGui::Text("  ");
            ImGui::SameLine();
            ImGui::Text("Tape %d:", i + 1);
            ImGui::SameLine();
            if (tape_reader.has_tape())
            {
                ImGui::Text("Loaded at ");
                ImGui::SameLine();
                ImGui::Text("%s", tape_reader.tape_location().as_string().c_str());

                switch (tape_reader.mode())
                {
                case tape_reader_t::eTapeMode::kForwardErase:
                    ImGui::Text("FORWARD ERASE");
                    break;
                case tape_reader_t::eTapeMode::kForward:
                    ImGui::Text("FORWARD");
                    break;
                case tape_reader_t::eTapeMode::kForwardHighSpeed:
                    ImGui::Text("FORWARD HIGH SPEED");
                    break;
                case tape_reader_t::eTapeMode::kReverse:
                    ImGui::Text("REVERSE");
                    break;
                case tape_reader_t::eTapeMode::kStop:
                    ImGui::Text("STOPPED");
                    break;
                case tape_reader_t::eTapeMode::kRewind:
                    ImGui::Text("REWIND");
                    break;
                    ;
                }
                if (ImGui::Button(std::format("Eject##{}", i).c_str()))
                {
                    // tape.eject();
                }
                ImGui::SameLine();
                if (ImGui::Button(std::format("Rewind##{}", i).c_str()))
                {
                    // tape.load( ... );
                }
            }
            else
            {
                ImGui::Text("Unloaded");
                if (ImGui::Button("Load"))
                {
                    std::cout << "TODO LOAD TAPE" << std::endl;
                }
            }
        }
        // ImGui::Text("  Position: %d", tape.position());
        // ImGui::Text("  Next byte: %s", to_octal(tape.next()).c_str());
    ImGui::End();

    render_breakpoints( emu );
}

void render_emulator(emulator_t &emu)
{
    sIaw = emu.cpu().iaw();

    render_internals( emu );
    emu.render_crt();
    render_screen( emu.screen() );
    if (show_data)
        render_memory( emu.memory(), sAddrs );
    if (show_assembler)
        render_assembler(emu);
}

void run_emulator()
{
    emulator_t emu;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window* window = SDL_CreateWindow("Cogar C4/Singer 1501/ICL 1501 emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);

    ImGui_ImplOpenGL3_Init("#version 150"); // <- macOS + GL 3.2 core expects 150
    // ImGui_ImplOpenGL3_Init("#version 130");

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Create a fullscreen dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        // Create the dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();

        render_emulator(emu);

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
