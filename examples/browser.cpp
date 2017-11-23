/*
 * (c)2017 Michael Tesch. tesch1@gmail.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ 
#include <fstream>
#define CXXOPTS_NO_RTTI
#include "cxxopts.hpp"
#include "sdl-container.h"
#include "context.h"
#include "document.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


using namespace litehtml;
typedef litehtml::tstring string;

static bool readFileString(const string & filename, string & dst)
{
  std::ifstream file(filename.c_str());
  if (!file) {
    std::cerr << "unable to open '" << filename << "'\n";
    return false;
  }
  std::string str((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());
  dst = str;
  return true;
}

class browser_window {
public:
  browser_window(string master_css)
    : _main_window(nullptr),
      _gl_context(nullptr),
      _user_styles(0),
      _doc()
  {
    // load the master css for this window
    _context.load_master_stylesheet(master_css.c_str());
  }

  // create & parse the document
  bool load_str(string html)
  {
    _doc = litehtml::document::createFromString(html.c_str(), &_painter, &_context, _user_styles);
    return _doc.get() != nullptr;
  }

  // 
  bool load_url(string url)
  {
    return false;
  }

  bool draw()
  {
    if (!_doc)
      return false;

    // render
    int max_width = 1024;
    int best_width = _doc->render(max_width);

    // size of rendered document:
    std::cout << "width: " << _doc->width() << " height: " << _doc->height() << "\n";

    // draw
    uint_ptr hdc = 0; // some device context, passed to sdl_doc_container funcs
    position * clip = nullptr;
    int left = 0, top = 0;
    _doc->draw(hdc, left, top, clip);

    return true;
  }

  bool show()
  {
    if (_main_window)
      return false;

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
      SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
      dm.w = 1024;
      dm.h = 768;
    }

    int posx = SDL_WINDOWPOS_UNDEFINED;
    int posy = SDL_WINDOWPOS_UNDEFINED;
    int width = dm.w;
    int height = dm.h;

    _main_window = SDL_CreateWindow("", posx, posy, width, height,
                                    SDL_WINDOW_SHOWN
                                    | SDL_WINDOW_OPENGL
#if (defined(__APPLE__) && TARGET_OS_IPHONE) || defined(ANDROID)
                                    | SDL_WINDOW_BORDERLESS
#endif
                                    | SDL_WINDOW_ALLOW_HIGHDPI
                                    | SDL_WINDOW_RESIZABLE);
    if (!_main_window)
    {
      std::cerr << "Unable to create window: " << SDL_GetError() << "\n";
      return false;
    }

    _gl_context = SDL_GL_CreateContext(_main_window);
    SDL_GL_MakeCurrent(_main_window, _gl_context);

    //
    // see if rendering at a higher resolution than the events
    //
    int point_width, point_height;
    SDL_GetWindowSize(_main_window, &point_width, &point_height);
    SDL_GL_GetDrawableSize(_main_window, &width, &height);
    _xratio = (float)width / point_width;
    _yratio = (float)height / point_height;

    draw();

    return true;
  }

private:
  float _xratio;
  float _yratio;
  SDL_Window * _main_window;
  SDL_GLContext _gl_context;

  litehtml::context _context;
  sdl_doc_container _painter;
  litehtml::css * _user_styles;
  litehtml::document::ptr _doc;
};

int main(int argc, char * argv[])
{
  // Parse the command-line options
  cxxopts::Options options(argv[0], "browser");
  options.add_options()
    ("s,stylesheet",    "master stylesheet", cxxopts::value<std::string>(), "master.css")
    ("p,page",          "html page", cxxopts::value<std::string>(), "index.html")
    ("d,debug",         "set debug level", cxxopts::value<int>(), "level")
    ("v,verbose",       "be more verbose")
    ("V,version",       "print the version and exit")
    ("h,help",          "print help and exit")
    ;

  if (SDL_Init(SDL_INIT_VIDEO
               | SDL_INIT_EVENTS
#ifndef __EMSCRIPTEN__ //! \todo emscripten audio is broken, makes noise but not sound
               | SDL_INIT_TIMER
               | SDL_INIT_AUDIO
#endif
               ) != 0)
  {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << "\n";
    return -1;
  }
  atexit(SDL_Quit);

  //
  // Setup OpenGL context
  //
#if defined(TB_RENDERER_GLES_2)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#elif defined(TB_RENDERER_GL3)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

  //
#ifdef ENABLE_MSAA
  int msaa = g_prefs.GetInt("MSAA", 16);
  if (msaa > 1) {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
  }
#endif

  // load master stylesheet
  string stylesheet;
  readFileString(options["stylesheet"].as<string>(), stylesheet);

  // create window with master stylesheet
  browser_window window(stylesheet);

  // load the page 
  string html;
  readFileString(options["page"].as<string>(), html);
  window.load_str(html);

  // start ths show...
  window.show();

  return 0;
}
