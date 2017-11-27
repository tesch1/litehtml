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
#define CXXOPTS_NO_RTTI
#include "cxxopts.hpp"
#include "cxx_utils.hpp"
#include "sdl-container.h"
#include "context.h"
#include "document.h"

#include <SDL2/SDL.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_SIZES_H

using namespace litehtml;
typedef litehtml::tstring string;

class browser_window {
public:
  browser_window()
  {
  }

  bool show(string master_css)
  {
    if (_main_window)
      return false;

    if (_width <= 0 || _height <= 0) {
      SDL_DisplayMode dm;
      if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
      }
      else {
        _width = dm.w;
        _height = dm.h;
      }
    }

    int posx = SDL_WINDOWPOS_UNDEFINED;
    int posy = SDL_WINDOWPOS_UNDEFINED;

    _main_window = SDL_CreateWindow("", posx, posy, _width, _height,
                                    SDL_WINDOW_SHOWN
                                    | SDL_WINDOW_OPENGL
#if (defined(__APPLE__) && TARGET_OS_IPHONE) || defined(ANDROID)
                                    | SDL_WINDOW_BORDERLESS
#endif
                                    | SDL_WINDOW_ALLOW_HIGHDPI
                                    | SDL_WINDOW_RESIZABLE);
    if (!_main_window) {
      std::cerr << "Unable to create window: " << SDL_GetError() << "\n";
      return false;
    }

    _renderer = SDL_CreateRenderer(_main_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_renderer) {
      std::cerr << "Unable to create rendered: " << SDL_GetError() << "\n";
      return false;
    }

    // give it something to paint with
    _painter.set_renderer(_renderer);

    // get scaling (from ALLOW_HIGHDPI)
    SDL_RenderGetScale(_renderer, &_xratio, &_yratio);
    std::cout << "window : " << _width << " x " << _height
              << " ratios: " << _xratio << "x" << _yratio << "\n";

    // load the master css for this window
    _context.load_master_stylesheet(master_css.c_str());

    //draw(); ?
    return true;
  }

  // create & parse the document
  bool load_str(const string & urlbase, const string & html)
  {
    if (!_renderer)
      return false;

    // remember where the file was
    _painter.set_base_url(urlbase.c_str());

    // make the doc
    _doc = litehtml::document::createFromString(html.c_str(), &_painter, &_context, _user_styles);

    // render
    int best_width = _doc->render(_width);

    // size of rendered document:
    std::cout << "frame: " << _frame << " width: " << _doc->width() << " height: " << _doc->height()
              << " best width: " << best_width << "\n";

    return _doc.get() != nullptr;
  }

  // nothing yet
  bool load_url(const string & url)
  {
    string html;
    {
      string filename = url;
      std::cout << "loading html file: " << filename << "\n";
      if (!file_to_string(filename, html))
        return -1;
    }

    load_str(cxx_dirname(url), html);

    return false;
  }

  // draw the document
  bool draw()
  {
    if (!_doc || !_renderer)
      return false;

    // draw
    uint_ptr hdc = (uint_ptr)_renderer;
    litehtml::position clip;
    int left = -_scrollx;
    int top = -_scrolly;
    clip.width = 1024;
    clip.height = 768;
    clip.x = 0;
    clip.y = 0;

    /* Select the color for background clearing. */
    SDL_SetRenderDrawColor(_renderer, 164, 164, 164, 255);

    SDL_RenderClear(_renderer);
    _doc->draw(hdc, left, top, &clip);
    SDL_RenderPresent(_renderer);

    std::cout << "frame: " << _frame << "\n";
    _frame++;

    return true;
  }

  void eventloop()
  {
    SDL_Event event;
    _done = false;
    do {
      // handle events
      SDL_WaitEvent(&event);
      handle_event(event);
    } while (!_done);
  }

  void queue_redraw(Sint32 code = 0, void * data1 = nullptr, void * data2 = nullptr)
  {
    // queue a user event to cause the event loop to run
    SDL_Event event;
    SDL_UserEvent userevent;
    userevent.type = SDL_USEREVENT;
    userevent.code = code;
    userevent.data1 = data1;
    userevent.data2 = data2;
    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);
  }

  bool handle_event(SDL_Event & event)
  {
    switch (event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_F1:		
      case SDLK_F2:		
      case SDLK_F3:		
      case SDLK_F4:		
      case SDLK_F5:		
      case SDLK_F6:		
      case SDLK_F7:		
      case SDLK_F8:		
      case SDLK_F9:		
      case SDLK_F10:		
      case SDLK_F11:		
      case SDLK_F12:		
      case SDLK_LEFT:		
      case SDLK_UP:		
      case SDLK_RIGHT:	
      case SDLK_DOWN:		
      case SDLK_PAGEUP:	
      case SDLK_PAGEDOWN:	
      case SDLK_HOME:		
      case SDLK_END:		
      case SDLK_INSERT:	
      case SDLK_TAB:		
      case SDLK_DELETE:	
      case SDLK_BACKSPACE:	
      case SDLK_RETURN:	
      case SDLK_KP_ENTER:	
      case SDLK_ESCAPE:	
      case SDLK_MENU:
        break;
      }
      break;
    case SDL_TEXTEDITING:
      break;
    case SDL_TEXTINPUT:
      break;
    case SDL_FINGERMOTION:
    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
      break;
    case SDL_MOUSEMOTION:
      if (_doc) {
        position::vector redraw_boxes;
        int client_x = event.motion.x + _scrollx;
        int client_y = event.motion.y + _scrolly;
        if (_doc->on_mouse_over(event.motion.x, event.motion.y, client_x, client_y, redraw_boxes))
          queue_redraw();
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (_doc) {
        position::vector redraw_boxes;
        int client_x = event.motion.x + _scrollx;
        int client_y = event.motion.y + _scrolly;
        if (_doc->on_lbutton_up(event.motion.x, event.motion.y, client_x, client_y, redraw_boxes))
          queue_redraw();
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (_doc) {
        position::vector redraw_boxes;
        int client_x = event.motion.x + _scrollx;
        int client_y = event.motion.y + _scrolly;
        if (_doc->on_lbutton_down(event.motion.x, event.motion.y, client_x, client_y, redraw_boxes))
          queue_redraw();
      }
      break;
    case SDL_MOUSEWHEEL: {
      if (_doc) {
        _scrollx += event.wheel.x * 10;
        _scrolly += event.wheel.y * 10;
        int width, height;
        SDL_GetRendererOutputSize(_renderer, &width, &height);
        _scrollx = std::max(0, std::min(_doc->width() - width, _scrollx));
        _scrolly = std::max(0, std::min(_doc->height() - height, _scrolly));
        _painter.scroll_set(_scrollx, _scrolly);
        queue_redraw();
        std::cout << "scrolly:" << _scrolly << "," << _scrollx << "\n";
      }
    }                          
      break;
    case SDL_MULTIGESTURE:
      break;
    case SDL_SYSWMEVENT:
      break;
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_SHOWN:
        queue_redraw();
        break;
      case SDL_WINDOWEVENT_HIDDEN:
        break;
      case SDL_WINDOWEVENT_EXPOSED:
        queue_redraw();
        break;
      case SDL_WINDOWEVENT_MOVED:
        break;
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        int width, height;
        SDL_GetRendererOutputSize(_renderer, &width, &height);
        //SDL_GL_GetDrawableSize(_main_window, &width, &height);
        _xratio = (float)width / event.window.data1;
        _yratio = (float)height / event.window.data2;

        _doc->media_changed();
        _doc->render(width);

        queue_redraw();
      }
        break;
      case SDL_WINDOWEVENT_MINIMIZED:
        break;
      case SDL_WINDOWEVENT_MAXIMIZED:
        break;
      case SDL_WINDOWEVENT_RESTORED:
        break;
      case SDL_WINDOWEVENT_ENTER:
        break;
      case SDL_WINDOWEVENT_LEAVE:
        if (_doc) {
          position::vector redraw_boxes;
          if (_doc->on_mouse_leave(redraw_boxes))
            queue_redraw();
        }
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        break;
      case SDL_WINDOWEVENT_FOCUS_LOST:
        break;
      case SDL_WINDOWEVENT_CLOSE:
        break;
      default:
        break;
      }
      break;
    }
    case SDL_USEREVENT:
      draw();
      break;
#if 0 // SDL2 >=2.0.4
    case SDL_KEYMAPCHANGED:
      break;
    case SDL_AUDIODEVICEADDED:
      break;
    case SDL_AUDIODEVICEREMOVED:
      break;
#endif
    case SDL_QUIT:
      _done = true;
      break;
    default:
      break;
    }
    return true;
  }

  void set_size(int width, int height)
  {
    _width = width;
    _height = height;
  }

private:
  SDL_Window * _main_window = nullptr;
  SDL_Renderer * _renderer = nullptr;
  bool _done = false;
  int _width = 0;
  int _height = 0;
  int _frame = 0;
  int _scrollx = 0;
  int _scrolly = 0;
  float _xratio = 1;
  float _yratio = 1;

  litehtml::context _context;
  sdl_doc_container _painter;
  litehtml::css * _user_styles = nullptr;
  litehtml::document::ptr _doc;
};

FT_Library g_freetype;
static void done_freetype()
{
  FT_Done_FreeType(g_freetype);
  g_freetype = nullptr;
}

static void init_freetype()
{
  FT_Init_FreeType(&g_freetype);
}

int main(int argc, char * argv[])
{
  // Parse the command-line options
  cxxopts::Options options(argv[0], "browser");
  options.add_options()
    ("s,size",          "set the window size", cxxopts::value<string>(), "WxH")
    ("c,stylesheet",    "master stylesheet", cxxopts::value<std::string>(), "master.css")
    ("p,page",          "html page", cxxopts::value<std::string>(), "index.html")
    ("a,msaa",          "set the Multi-Sample AntiAliasing factor", cxxopts::value<int>(), "2-16")
    ("d,debug",         "set debug level", cxxopts::value<int>(), "level")
    ("v,verbose",       "be more verbose")
    ("V,version",       "print the version and exit")
    ("h,help",          "print help and exit")
    ;

  options.parse_positional("page");
  options.positional_help("[file.html]");
  try {
    options.parse(argc, argv);
  }
  catch (std::exception & ex) {
    std::cerr << "Parsing cmd line: " << ex.what() << "\n";
    return -1;
  }

  if (options.count("help")) {
    std::cout << options.help() << "\n";
    return 0;
  }

  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << "\n";
    return -1;
  }
  atexit(SDL_Quit);

  // init the font system
  init_freetype();
  atexit(&done_freetype);

  // create window with master stylesheet
  browser_window window;

  // set the window size
  if (options.count("size")) {
    string size = options["size"].as<string>();
    int width, height;
    if (2 == sscanf(size.c_str(), "%dx%d", &width, &height)) {
      std::cout << "size : " << width << " x " << height << "\n";
      window.set_size(width, height);
    }
  }

  // open the window
  // load master stylesheet
  string stylesheet;
  file_to_string("../include/master.css", stylesheet);
  if (options.count("stylesheet")) {
    string filename = options["stylesheet"].as<string>();
    if (!file_to_string(filename, stylesheet))
      return -1;
  }
  window.show(stylesheet);

  // load the page
  window.load_url(options["page"].as<string>());

  // run ths show...
  window.eventloop();

  return 0;
}
