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
#pragma once
#include "html.h"
#include <SDL2/SDL.h>
#include <map>

bool file_to_string(const litehtml::tstring & filename, litehtml::tstring & dst);

class sdl_doc_container : public litehtml::document_container
{
 private:
  SDL_Renderer * _renderer = nullptr;
  std::map<litehtml::tstring, std::pair<SDL_Surface *, SDL_Texture *> > _images;
  litehtml::tstring _base_url;
  int _x = 0;
  int _y = 0;

 public:
  sdl_doc_container();
  ~sdl_doc_container();

  void set_renderer(SDL_Renderer * renderer);
  void set_renderer_draw_color(const litehtml::web_color & color);
  void make_url(const litehtml::tchar_t * src, const litehtml::tchar_t * baseurl, litehtml::tstring & url);
  void scroll_delta(int x, int y) { _x += x; _y += y; }
  void scroll_set(int x, int y) { _x = x; _y = y; }

  // The litehtml 'document_container' painting interface...
  virtual litehtml::uint_ptr create_font(const litehtml::tchar_t * faceName, int size, int weight,
                                         litehtml::font_style italic, unsigned int decoration,
                                         litehtml::font_metrics * fm);
  virtual void delete_font(litehtml::uint_ptr hFont);
  virtual int text_width(const litehtml::tchar_t * text, litehtml::uint_ptr hFont);
  virtual void draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t * text, litehtml::uint_ptr hFont,
                         litehtml::web_color color, const litehtml::position & pos);
  virtual int pt_to_px(int pt);
  virtual int get_default_font_size() const;
  virtual const litehtml::tchar_t * get_default_font_name() const;
  virtual void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker & marker);
  virtual void load_image(const litehtml::tchar_t * src, const litehtml::tchar_t * baseurl, bool redraw_on_ready);
  virtual void get_image_size(const litehtml::tchar_t * src, const litehtml::tchar_t * baseurl, litehtml::size & sz);
  virtual void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint & bg);
  virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders & borders,
                            const litehtml::position & draw_pos, bool root);

  virtual void set_caption(const litehtml::tchar_t * caption);
  virtual void set_base_url(const litehtml::tchar_t * base_url);
  virtual void link(const std::shared_ptr < litehtml::document > &doc, const litehtml::element::ptr & el);
  virtual void on_anchor_click(const litehtml::tchar_t * url, const litehtml::element::ptr & el);
  virtual void set_cursor(const litehtml::tchar_t * cursor);
  virtual void transform_text(litehtml::tstring & text, litehtml::text_transform tt);
  virtual void import_css(litehtml::tstring & text, const litehtml::tstring & url, litehtml::tstring & baseurl);
  virtual void set_clip(const litehtml::position & pos, const litehtml::border_radiuses & bdr_radius, bool valid_x,
                        bool valid_y);
  virtual void del_clip();
  virtual void get_client_rect(litehtml::position & client) const;
  virtual std::shared_ptr < litehtml::element > create_element(const litehtml::tchar_t * tag_name,
                                                               const litehtml::string_map & attributes,
                                                               const std::shared_ptr < litehtml::document > &doc);

  virtual void get_media_features(litehtml::media_features & media) const;
  virtual void get_language(litehtml::tstring & language, litehtml::tstring & culture) const;
  virtual litehtml::tstring resolve_color(const litehtml::tstring & color) const;
};
