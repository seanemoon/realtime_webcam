#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
#undef __STDC_CONSTANT_MACROS

namespace {

int GetVideoStream(AVFormatContext* context) {
  std::set<unsigned> video_streams;
  for (unsigned i = 0; i < context->nb_streams; ++i) {
    if (context->streams[i]->codecpar->codec_type
        == AVMEDIA_TYPE_VIDEO) {
      video_streams.insert(i);
    }
  }
  if (video_streams.size() != 1) {
    return -1;
  } else {
    return *video_streams.begin();
  }
}


AVFormatContext* GetWebcamContext(
    const char* device,
    const char* format) {
  av_register_all();
  avdevice_register_all();
  avformat_network_init();

  AVInputFormat* input_format = av_find_input_format(format);
  if (input_format == nullptr) {
    std::cerr << "Failed to find input format: " << format << std::endl;
    return nullptr;
  }

  AVFormatContext *input_format_context = avformat_alloc_context();
  if (input_format_context == nullptr) {
    std::cerr << "Failed to allocate input format context" << std::endl;
    return nullptr;
  }
// input_format_context->width = kWidth;
//  input_format_context->height = kHeight;


  // av_dict_set will allocate an AVDictionary and update the pointer if nullptr
  // is passed.
  AVDictionary* dictionary = nullptr;

  // Request MJPEG stream from the webcam (instead of default raw format).
  input_format_context->video_codec_id = AV_CODEC_ID_MJPEG;

  av_dict_set(&dictionary, "video_size", "640x480", 0);
  av_dict_set(&dictionary, "framerate", "30", 0);

  if (avformat_open_input(
        &input_format_context, device, input_format, &dictionary)
      != 0) {
    std::cerr << "Failed to open input device: " << device << std::endl;
    return nullptr;
  }

// PrintStreamInfo(input_format_context);

  return input_format_context;
}


}

int main(int argc, char*argv[]) {
  static constexpr char kDefaultDevice[] = "/dev/video0";
  const char* device = argc > 1 ? argv[1] : kDefaultDevice;
  static constexpr char kDefaultFormat[] = "video4linux2";
  const char* format = argc > 2 ? argv[2] : kDefaultFormat;

  AVFormatContext* webcam = GetWebcamContext(device, format);
  if (webcam == nullptr) {
    std::cerr << "Failed to open webcam." << std::endl;
  }
  int video_stream = GetVideoStream(webcam);
  if (video_stream == -1) {
    std::cerr << "Failed to get video stream for webcam." << std::endl;
    return -1;
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Renderer* renderer;
  SDL_Window* window;
  SDL_CreateWindowAndRenderer(
      640, 480, 0, &window, &renderer);
  IMG_Init(IMG_INIT_JPG);

  AVPacket packet;
  while (av_read_frame(webcam, &packet) == 0) {
    if (packet.stream_index != video_stream) {
      std::cerr << "Packet is not for video stream." << std::endl;
      continue;
    }
    SDL_RWops* rw = SDL_RWFromMem(packet.data, packet.size);
    if (rw == nullptr) {
      std::cerr << "Failed to convert packet to SDL_RWops";
    }
    SDL_Surface* img = IMG_LoadTyped_RW(rw, /*freesrc=*/1, "JPEG");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);
    SDL_RenderCopyEx(renderer, texture, nullptr, nullptr, 0, nullptr, SDL_FLIP_HORIZONTAL);
    SDL_RenderPresent(renderer);
    av_free_packet(&packet);
    SDL_DestroyTexture(texture);

    SDL_Event event;
    bool quit = false;
    while (SDL_PollEvent(&event) != 0) {
      /* an event was found */
      switch (event.type) {
      /* close button clicked */
      case SDL_QUIT:
        quit = true;
        break;

      /* handle the keyboard */
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
          case SDLK_q:
            quit = true;
            break;
        }
        break;
      }
    }
    if (quit) break;
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  avformat_free_context(webcam);
  return 0;
}
