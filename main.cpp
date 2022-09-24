#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_sdl.h"
#include "external/imgui/backends/imgui_impl_sdlrenderer.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"


SDL_Renderer *renderer;
SDL_Window *window;
SDL_Event event;


struct CharSpriteMap
{
    char c;
    unsigned int sprite_sheet_row; // this assumes each row only contains 1 letter animation.
};

struct SpriteSheet
{
    SDL_Texture *texture;
    size_t n_rows;
    size_t n_cols;
    size_t cell_size;
};


struct AnimatedFontSprite
{
    std::vector<CharSpriteMap> char_sprite_maps;

    unsigned int char_pixel_width;
    float seconds_per_frame;
    float accumulator;
    unsigned int curr_frame;
    SpriteSheet sprite_sheet;
    std::vector<std::string> lines;
    int color_mod[3];

    unsigned int n_frames()
    {
        return sprite_sheet.n_cols;
    }

    void increment(float delta_time_seconds)
    {
        accumulator += delta_time_seconds;
        if (accumulator >= seconds_per_frame)
        {
            accumulator = 0.0f;
            curr_frame++;
        }
        if (curr_frame >= n_frames())
        {
            curr_frame = 0;
        }
    }
};


void AnimatedFontSprite_increment(AnimatedFontSprite &animated_font_sprite, float delta_time_seconds)
{
    animated_font_sprite.accumulator += delta_time_seconds;
    if (animated_font_sprite.accumulator >= animated_font_sprite.seconds_per_frame)
    {
        animated_font_sprite.accumulator = 0.0f;
        animated_font_sprite.curr_frame++;
    }
    if (animated_font_sprite.curr_frame >= animated_font_sprite.n_frames())
    {
        animated_font_sprite.curr_frame = 0;
    }
}

SDL_Rect AnimatedFontSprite_generate_src_rect_for_char(char c, AnimatedFontSprite font_sprite)
{
    bool found = false;
    unsigned int sprite_sheet_row;
    SDL_Rect src_rect;

    for (CharSpriteMap char_sprite_map: font_sprite.char_sprite_maps)
    {
        if (toupper(c) == toupper(char_sprite_map.c))
        {
            found = true;
            sprite_sheet_row = char_sprite_map.sprite_sheet_row;
            break;
        }
    }

    const unsigned int not_found_char_row = 3;

    if (found)
    {
        src_rect.x = font_sprite.sprite_sheet.cell_size * font_sprite.curr_frame;
        src_rect.y = font_sprite.sprite_sheet.cell_size * sprite_sheet_row;
        src_rect.w = src_rect.h = font_sprite.sprite_sheet.cell_size;
    }
    else
    {
        src_rect.x = font_sprite.sprite_sheet.cell_size * font_sprite.curr_frame;
        src_rect.y = font_sprite.sprite_sheet.cell_size * not_found_char_row;
        src_rect.w = src_rect.h = font_sprite.sprite_sheet.cell_size;
    }

    return src_rect;
}


void AnimatedFontSprite_render(int posx, int posy, AnimatedFontSprite animated_font_sprite)
{
    SDL_SetTextureColorMod(animated_font_sprite.sprite_sheet.texture, animated_font_sprite.color_mod[0],
                           animated_font_sprite.color_mod[1], animated_font_sprite.color_mod[2]);

    for (int l = 0; l < animated_font_sprite.lines.size(); l++)
    {
        int rendery = posy + (l * animated_font_sprite.char_pixel_width);
        std::string line = animated_font_sprite.lines[l];

        for (int i = 0; i < line.size(); i++)
        {
            char c = line[i];
            int renderx = posx + (i * animated_font_sprite.char_pixel_width);

            SDL_Rect src_rect = AnimatedFontSprite_generate_src_rect_for_char(c, animated_font_sprite);
            SDL_Rect dest_rect = {
                    renderx,
                    rendery,
                    (int) animated_font_sprite.char_pixel_width,
                    (int) animated_font_sprite.char_pixel_width
            };

            SDL_RenderCopy(renderer, animated_font_sprite.sprite_sheet.texture, &src_rect, &dest_rect);
        }
    }

}


SpriteSheet
SpriteSheet_init(const std::string &filename, unsigned int n_rows, unsigned int n_cols, unsigned int cell_size)
{
    int req_format = STBI_rgb_alpha;
    int image_w, image_h, image_channels;
    unsigned char *image_data = stbi_load(filename.c_str(), &image_w, &image_h, &image_channels, req_format);


    int depth, pitch;
    SDL_Surface *image_surface;
    Uint32 pixel_format;

    depth = 32;
    pitch = 4 * image_w;
    pixel_format = SDL_PIXELFORMAT_RGBA32;

    image_surface = SDL_CreateRGBSurfaceWithFormatFrom(image_data, image_w, image_h, depth, pitch, pixel_format);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image_surface);
//    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MOD);
    if (texture == NULL)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
    }

    SDL_FreeSurface(image_surface);
    stbi_image_free(image_data);

    SpriteSheet sprite_sheet;
    sprite_sheet.texture = texture;
    sprite_sheet.n_rows = n_rows;
    sprite_sheet.n_cols = n_cols;
    sprite_sheet.cell_size = cell_size;

    return sprite_sheet;
}


int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1440, 1440, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // IMGUI Setup
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    AnimatedFontSprite animated_font_sprite;
    animated_font_sprite.sprite_sheet = SpriteSheet_init("fontSpriteSheet.png", 4, 4, 200);
    animated_font_sprite.curr_frame = 0;
    animated_font_sprite.accumulator = 0.0f;
    animated_font_sprite.seconds_per_frame = 0.1;
    animated_font_sprite.lines.push_back("LORD MAZE");
    animated_font_sprite.lines.push_back("RULER OF MAZES");
    animated_font_sprite.color_mod[0] = 255;
    animated_font_sprite.color_mod[1] = 255;animated_font_sprite.color_mod[2] = 255;
    animated_font_sprite.char_pixel_width = 100;
    for (char i = 0; i <= 26; i++)
    {
        char c = 'A' + i;
        unsigned int row = i;
        animated_font_sprite.char_sprite_maps.push_back((CharSpriteMap) {c, row});

    }
    animated_font_sprite.char_sprite_maps.push_back((CharSpriteMap) {' ', 27});


    double delta_time_in_sec = 0.0f;
    double frame_start_seconds = 0.0f;
    double frame_end_seconds = 0.0f;


    bool quit = false;
    while (!quit)
    {
        delta_time_in_sec = frame_end_seconds - frame_start_seconds;
        frame_start_seconds = (double) SDL_GetTicks() / 1000.0f;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Animated Font Sprite"))
        {
            ImGui::SliderInt("R", (int *) &animated_font_sprite.color_mod[0], 0, 255);
            ImGui::SliderInt("G", (int *) &animated_font_sprite.color_mod[1],  0, 255);
            ImGui::SliderInt("B", (int *) &animated_font_sprite.color_mod[2], 0, 255 );

//            ImGui::InputInt("G", (int *) &animated_font_sprite.color_mod.g);
//            ImGui::InputInt("B", (int *) &animated_font_sprite.color_mod.b);

            //            // add line
            if(ImGui::Button("Add Line"))
            {
                animated_font_sprite.lines.push_back("");
            }
            for(int i = 0; i < animated_font_sprite.lines.size(); i++)
            {
                char label[4];
                sprintf(label, "%d", i);
                ImGui::InputText(label, &animated_font_sprite.lines[i]);
            }
        }
        ImGui::End();

        animated_font_sprite.increment(delta_time_in_sec);
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        AnimatedFontSprite_render(10, 10, animated_font_sprite);


        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        frame_end_seconds = (double) SDL_GetTicks() / 1000.0f;
    }


    std::cout << "Hello, World!" << std::endl;
    return 0;
}
