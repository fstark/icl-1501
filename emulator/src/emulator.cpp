#include "utils.hpp"
#include <cstdint>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <bit>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "addrs.hpp"
#include "iw.hpp"
#include "disassembler.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "assembler.hpp"
#include "binary.hpp"


void load_bootstrap(memory_t &memory)
{
    // Load the bootstrap code into memory starting at P01-000
    memory.copy(
        addrs_t("P01-000"),
        vector_from_octal_pairs("201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030"));
}

void test_disassemble_memory(const std::string &adrs_string, const std::string &data_string)
{
    addrs_t adrs(adrs_string);
    auto data = vector_from_octal_pairs(data_string);

    memory_t memory;
    memory.copy(adrs, data);

    std::cout << "Testing disassembly" << std::endl;

    disassembler_t disassembler;

    for (size_t i = 0; i != data.size() / 2; i++)
    {
        iw_t w = memory.get_instruction(adrs);
        std::cout << adrs.as_string() << ": " << w.as_octal() << "      " << disassembler.disassemble(w) << std::endl;
        adrs = adrs.next_instruction();
    }
}

#include "crt.hpp"

void display( const crt_t::screen_buffer_t screen )
{
    // std::cout << "\033[H"; // Move cursor to home position
    std::cout << "SCREEN:" << std::endl;
    for (auto row=0;row!=crt_t::matrix_height_*crt_t::screen_lines_;row++)
    {
        for (auto col=0;col!=crt_t::matrix_width_*crt_t::screen_columns_;col++)
        {
            std::cout << screen[row][col];
        }
        std::cout << std::endl;
    }
    std::cout << "-------" << std::flush;

    std::string s;
    std::cin >> s;
}

void load_font(memory_t &memory)
{
    memory.copy(
        addrs_t("P04-000"),
        vector_from_octal(
            //    -   _   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   ,   #   @   -   %   $   *   .   <   >   /   (   )   ?   c   =   "   !   '   :   ;   -   \   &   |
            "000 010 010 076 000 162 042 030 047 074 141 066 106 174 177 076 177 177 177 076 177 000 060 177 177 177 177 177 177 076 177 042 001 077 007 177 143 007 141 000 024 014 010 143 044 052 000 000 101 040 034 000 000 034 024 000 000 000 000 000 100 002 060 000 "
            "000 010 010 101 102 111 101 024 105 112 021 111 111 022 111 101 101 111 011 101 010 101 100 010 100 002 006 101 011 101 011 105 001 100 030 040 024 010 121 130 167 062 010 023 052 034 140 010 042 020 042 000 002 042 024 007 000 007 000 000 100 002 116 000 "
            "000 010 176 101 177 111 111 022 105 111 011 111 111 021 111 101 101 111 011 101 010 177 100 024 100 014 010 101 011 121 031 111 177 100 140 030 010 170 111 070 000 052 010 010 177 076 150 024 024 010 101 101 001 177 024 000 137 007 066 133 100 002 131 177 "
            "000 010 010 101 100 111 111 177 105 111 005 111 051 022 111 101 101 101 001 111 010 101 100 042 100 002 060 101 011 041 051 121 001 100 030 040 024 010 105 000 167 072 010 144 052 034 000 042 010 004 000 042 131 042 024 007 000 000 066 073 100 002 046 177 "
            "000 010 010 076 000 106 066 020 071 060 003 066 036 174 066 042 076 101 001 171 177 000 077 101 100 177 177 177 006 136 106 042 001 077 007 177 143 007 103 000 024 074 010 143 022 052 000 101 000 002 000 034 006 000 024 000 000 000 000 000 100 016 120 000"
        ));
    memory.copy(
        addrs_t("P06-000"),
        vector_from_octal(
            //    -   _   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   ,   #   @   -   %   $   *   .   <   >   /   (   )   ?   c   =   "   !   '   :   ;   -   \   &   |
            "000 010 010 076 000 162 042 030 047 074 141 066 106 174 177 076 177 177 177 076 177 000 060 177 177 177 177 177 177 076 177 042 001 077 007 177 143 007 141 000 024 014 010 143 044 052 000 000 101 040 034 000 000 034 024 000 000 000 000 000 100 002 060 000 "
            "000 010 010 101 102 111 101 024 105 112 021 111 111 022 111 101 101 111 011 101 010 101 100 010 100 002 006 101 011 101 011 105 001 100 030 040 024 010 121 130 167 062 010 023 052 034 140 010 042 020 042 000 002 042 024 007 000 007 000 000 100 002 116 000 "
            "000 010 176 101 177 111 111 022 105 111 011 111 111 021 111 101 101 111 011 101 010 177 100 024 100 014 010 101 011 121 031 111 177 100 140 030 010 170 111 070 000 052 010 010 177 076 150 024 024 010 101 101 001 177 024 000 137 007 066 133 100 002 131 177 "
            "000 010 010 101 100 111 111 177 105 111 005 111 051 022 111 101 101 101 001 111 010 101 100 042 100 002 060 101 011 041 051 121 001 100 030 040 024 010 105 000 167 072 010 144 052 034 000 042 010 004 000 042 131 042 024 007 000 000 066 073 100 002 046 177 "
            "000 010 010 076 000 106 066 020 071 060 003 066 036 174 066 042 076 101 001 171 177 000 077 101 100 177 177 177 006 136 106 042 001 077 007 177 143 007 103 000 024 074 010 143 022 052 000 101 000 002 000 034 006 000 024 000 000 000 000 000 100 016 120 000"
        ));
    for (addrs_t a = addrs_t{"P06-000"}; a!=addrs_t{"P07-177"}; a = a+1)
    {
        memory.set( a, memory[a] ^ 0xff ); // Inverted font
    }
    memory.copy("P10-000", vector_from_ascii( " -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"));
    memory.copy("P10-100", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|",true));
    memory.copy("P10-200", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|"));
    memory.copy("P10-300", vector_from_ascii(" -_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,#@-%$*.<>/()?c=\"!':;-\\&|",true));
}

void test_cpu_t()
{
    memory_t memory;
    load_bootstrap(memory);

    load_font(memory);

    io_t io( memory );
    io.execute(iw_t{0174, 0b00000010}); // IOC C#4 020 ; Screen in P01-000, no underline

    cpu_t cpu(memory, io);
    cpu.reset();
    std::cout << "Bootstrap loaded into memory." << std::endl;
    while (1)
    {
        crt_t::screen_buffer_t screen = {};
        io.crt().render( screen);
        display( screen );
        cpu.step();
    }
    std::cout << "CPU step executed." << std::endl;
}

class emulator_t
{
    memory_t memory_;
    io_t io_;
    cpu_t cpu_;
    crt_t::screen_buffer_t screen_;
public:
    emulator_t() : memory_(), io_(memory_), cpu_(memory_, io_)
    {
        load_bootstrap(memory_);
        load_font(memory_);
        // io_.execute(iw_t{0174, 0b00001010}); // IOC C#4 020 ; Screen in P01-000, underline
        cpu_.execute(iw_t{0174, 0b01001000}); // IOC C#4 120 ; Screen in P10-000, underline
        cpu_.reset();

        render_crt();
    }

    void render_crt()
    {
        /*
        ORG P04-000
        DBO 377-377
         */
        if (memory_.changed())
        {
            std::cout << "CRT rendered." << std::endl;
            io_.crt().render( screen_ );
        }
    }

    cpu_t &cpu() { return cpu_; }
    io_t &io() { return io_; }
    memory_t &memory() { return memory_; }
    crt_t::screen_buffer_t &screen() { return screen_; }
};

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <format>

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

void render_addrs(addrs_t addrs)
{
    std::string label = addrs.as_string();
    ImGui::Text("%s", label.c_str());
}

void render_addrs(const char *label, addrs_t addrs)
{
    ImGui::Text("%s", label);
    ImGui::SameLine();
    render_addrs(addrs);
}

const int kID_PC = 0;
const int kID_SP = 1;
const int kID_CRT = 2;
const int kID_FONT = 3;
const int kID_ALT_FONT = 4;

const int kID_SP_BASE = 1000;

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
                    ImGui::Text("%s", to_octal(memory[a]).c_str());
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

void render_screen( const crt_t::screen_buffer_t &screen)
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

void render_internals( emulator_t &emu )
{
    auto &cpu = emu.cpu();

    ImGui::Begin("CPU State");

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

    if (ImGui::CollapsingHeader("CPU"))
    {
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
        for (int i = 0; i < 8; ++i) {
            if (i == cpu.sp())
                ImGui::Text( "*" );
            else
                ImGui::Text( " " );
            ImGui::SameLine();
            render_active_addrs(cpu.sp_base(i), kID_SP + 1 + i*2);
            ImGui::SameLine(80);
            render_active_addrs(cpu.memory().get_addrs(cpu.sp_base(i)), kID_SP + 1 + i*2 + 1);
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
        ImGui::Text("R#%d", i );
        ImGui::Text("%s", to_octal(cpu.index_register(i)).c_str());
        ImGui::EndGroup();
        if (i < 8) ImGui::SameLine();
        }
    }

    if (ImGui::CollapsingHeader("CRT"))
    {
        auto crt = emu.io().crt();

        render_active_addrs("  Screen: ", crt.screen(), kID_CRT);
        render_active_addrs("    Font: ", crt.font(), kID_FONT);
        render_active_addrs("Alt Font: ", crt.alt_font(), kID_ALT_FONT);
    }

    if (ImGui::CollapsingHeader("Tapes"))
    {
        auto io = emu.io();

        for (int i=0;i!=io.tape_count();i++)
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
                ImGui::Text( "Loaded at ");
                ImGui::SameLine();
                ImGui::Text("%s",tape_reader.tape_location().as_string().c_str());

                switch (tape_reader.mode())
                {
                    case tape_reader_t::eTapeMode::kForwardErase:
                        ImGui::Text( "FORWARD ERASE" );
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

            // ImGui::Text("  Position: %d", tape.position());
            // ImGui::Text("  Next byte: %s", to_octal(tape.next()).c_str());
        }
    }

ImGui::End();
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

void cmd_assemble(const std::string &source_file, const std::string &output_file = "")
{
    std::ifstream source(source_file);
    if (!source)
    {
        throw std::runtime_error("Failed to open source file: " + source_file);
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(source)),
                       std::istreambuf_iterator<char>());

    std::ostream *out;
    std::ofstream output;
    if (output_file.empty()) {
        out = &std::cout;
    } else {
        output.open(output_file, std::ios::binary);
        if (!output)
            throw std::runtime_error("Failed to open output file: " + output_file);
        out = &output;
    }

    try {
        binary_t binary = assemble(content);
        *out << to_string(binary);
    }
    catch (const std::exception &e) {
        std::cerr << "Assembly failed: " << e.what() << std::endl;
        throw;
    }
}

void cmd_disassemble(const std::string &source_file, const std::string &output_file = "")
{
    std::ifstream source(source_file, std::ios::binary);
    if (!source)
    {
        throw std::runtime_error("Failed to open source file: " + source_file);
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(source)),
                       std::istreambuf_iterator<char>());
    
    std::ostream *out;
    std::ofstream output;
    if (output_file.empty()) {
        out = &std::cout;
    } else {
        output.open(output_file);
        if (!output)
            throw std::runtime_error("Failed to open output file: " + output_file);
        out = &output;
    }

    try {
        // Parse the content as a binary_t
        binary_t binary = binary_from_string(content);
        
        // Create disassembler and generate disassembly
        disassembler_t disassembler;
        std::string disassembly = disassemble_binary(disassembler, binary);
        
        // Output the result
        *out << disassembly;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error disassembling file: " << e.what() << std::endl;
        throw;
    }
}

void test_assembler1( iw_t iw )
{
    iw_t iw2{0,0};

    disassembler_t disassembler;
    auto str = " "s+disassembler.disassemble(iw);
    std::cout << iw.as_octal() << " =>" << str << " => " << std::flush;
    assembler_t assembler;
    if (!assembler.assemble(str, iw2))
    {
        std::cout << str << " does not assemble " << std::endl;
            throw std::runtime_error("Assembler round trip failed");
    }
    else
        if (iw!= iw2)
        {
            std::cout << str << " does not assemble to " << iw.as_octal() << " but to " << iw2.as_octal() << std::endl;
            throw std::runtime_error("Assembler round trip failed");
        }
    std::cout << iw2.as_octal() << std::endl;
}

void test_assembler()
{
    iw_t iw{0201,0030};
    for (int i=0;i!=65536;i++)
    {
        iw.set_word(i);
        // std::cout << iw.as_octal() << " " << std::flush;
        test_assembler1(iw);
    }
}

void run_tests()
{
    // std::string adrs = "P00-030";
    // std::string data = "201-030 170-007 231-002 341-230 111-003 170-016 170-005 100-030";

    // test_addrs_t();
    // test_memory_t();
    // test_iw_t();
    // test_disassemble_memory(adrs, data);

    // test_cpu_t();

    test_assembler();
}


int main(int argc, char **argv)
{
    if (argc >= 2) {
        std::string mode = argv[1];
        if (mode == "-d" && argc >= 3) {
            std::string infile = argv[2];
            std::string outfile = (argc >= 4) ? argv[3] : "";
            cmd_disassemble(infile, outfile);
            return 0;
        } else if (mode == "-a" && argc >= 3) {
            std::string infile = argv[2];
            std::string outfile = (argc >= 4) ? argv[3] : "";
            cmd_assemble(infile, outfile);
            return 0;
        } else if (mode == "-t") {
            run_tests();
            return 0;
        } else {
            std::cerr << "Usage:\n";
            std::cerr << "  " << argv[0] << "            # run emulator\n";
            std::cerr << "  " << argv[0] << " -d infile [outfile]  # disassemble\n";
            std::cerr << "  " << argv[0] << " -a infile [outfile]  # assemble\n";
            std::cerr << "  " << argv[0] << " -t  # run tests\n";
            return 1;
        }
    }
    run_emulator();
    return 0;
}
