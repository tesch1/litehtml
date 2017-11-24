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

#include <iostream>
#include "sdl-container.h"
#include "thirdparty/butf8.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#define SDL_STBIMAGE_IMPLEMENTATION
#include "thirdparty/SDL_stbimage.h"

#ifndef LITEHTML_UTF8
#error "utf8!"
#endif

extern FT_Library g_freetype;

std::ostream & operator <<(std::ostream & out, SDL_Rect const & r)
{
  out << "[" << r.x << "," << r.y << "  " << r.w << "x" << r.h << "]";
  return out;
}

std::ostream & operator <<(std::ostream & out, litehtml::web_color const & c)
{
  out << "rgba(" << (int)c.red << "," << (int)c.green << "," << (int)c.blue << "," << (int)c.alpha << ")";
  return out;
}

SDL_Rect conv(const litehtml::position & pos)
{
  SDL_Rect dst;
  dst.x = pos.x;
  dst.y = pos.y;
  dst.w = pos.width;
  dst.h = pos.height;
  return dst;
}

class image {
public:
  image() {}
  ~image() {}

  bool load(const char * path) {
    return false;
  }

  int width() { return _w; }
  int height() { return _h; }

private:
  SDL_Surface * _surface;
  int _w, _h;
};

class ftfont {
public:
  static const int FONT_STYLE_ITALIC = 0x01;
  static const int FONT_STYLE_STRIKETHROUGH = 0x02;
  static const int FONT_STYLE_UNDERLINE = 0x04;

  static ftfont * CreateFont(const char * file, int size, int style)
  {
    // load the font desc into memory
    FT_Face face;
    FT_Error err = FT_New_Face(g_freetype, file, 0, &face);
    if (!err) {
      err = FT_Set_Char_Size(face, size * 64, size * 64, 72, 0);
      if (err)
        std::cerr << "FT_Set_Char_Size ERR: " << err << "\n";

      std::cout << "loaded " << file << ":" << size
                << " style: " << face->style_name
                << " num_glyphs: " << face->num_glyphs
                << " units_per_EM: " << face->units_per_EM
                << " num_fixed_sizes: " << face->num_fixed_sizes << "\n";

    }
    else
      std::cerr << "FT_New_Face ERR: " << err << "\n";

    ftfont * font = new ftfont();
    font->_face = face;
    //font->_size = size;
    return font;
  }

  ftfont() {
  }

  ~ftfont() {
    FT_Done_Face(_face);
  }

  int ascent() { return _face->ascender; }
  int descent() { return _face->descender; }
  int lineheight() { return _face->height; }
  int height() { return _face->height; }

  int text_width(const litehtml::tchar_t * text)
  {
    uint32_t ch;
    int width = 0;
    int er = 0;
    //static void * cur = (void *)text;
    //for (cur = utf8_decode(cur, &ch, &er); ch && !er; cur = utf8_decode(cur, &ch, &er)) {
    while ((ch = getUTF8Next(text))) {

      FT_Error err;
      FT_UInt gindex = FT_Get_Char_Index(_face, ch);
      if (0 != gindex && !(err = FT_Load_Glyph(_face, gindex, FT_LOAD_RENDER))) {
        FT_GlyphSlot slot = _face->glyph;
        //const float PTS_PER_UNIT = 96;
        //std::cout << "cp:" << ch << " ha:" << slot->metrics.horiAdvance << "\n";
        width += slot->metrics.horiAdvance / 64.f;
      }
      else {
        std::cerr << "text_width err: " << err << " gi=" << gindex << " e=" << er << "\n";
      }
    }
    return width;
  }

private:
  //FT_Size _size;
  FT_Face _face;
  int _style;
};

sdl_doc_container::sdl_doc_container()
{
}

void sdl_doc_container::set_renderer(SDL_Renderer * renderer)
{
  if (_renderer != renderer) {
    // flush any context associated with old renderer
  }

  _renderer = renderer;
}

void sdl_doc_container::setrendererdrawcolor(const litehtml::web_color & color)
{
  if (!_renderer)
    return;
  SDL_SetRenderDrawColor(_renderer, color.red, color.green, color.blue, color.alpha);
}

void sdl_doc_container::make_url(const litehtml::tchar_t * src,
                                 const litehtml::tchar_t * baseurl,
                                 litehtml::tstring & url)
{
  //std::cout << "#make_url '" << src << "' | '" << (baseurl ? baseurl : "0") << " | " << _base_url << "'\n";

  if (!baseurl) {
    url = _base_url + "/" + src;
  }
  else {
    url = litehtml::tstring(baseurl) + "/" + src;
  }
}

litehtml::uint_ptr
sdl_doc_container::create_font(const litehtml::tchar_t * faceName,
                               int size,
                               int weight,
                               litehtml::font_style italic,
                               unsigned int decoration,
                               litehtml::font_metrics * fm)
{
  litehtml::string_vector fonts;
  litehtml::split_string(faceName, fonts, ",");
  litehtml::trim(fonts[0]);

  ftfont * font = nullptr;

  int style = 0;
  if (italic == litehtml::fontStyleItalic)
    style |= ftfont::FONT_STYLE_ITALIC;
  if ((decoration & litehtml::font_decoration_linethrough))
    style |= ftfont::FONT_STYLE_STRIKETHROUGH;
  if ((decoration & litehtml::font_decoration_underline))
    style |= ftfont::FONT_STYLE_UNDERLINE;

  //std::cout << "#create_font " << faceName << "." << size << "." << weight << "." << style << "\n";

  for (litehtml::string_vector::iterator i = fonts.begin(); i != fonts.end(); i++) {
    //std::cout << " trying '" << i->c_str() << "'\n";
    font = ftfont::CreateFont((*i + ".ttf").c_str(), size, style);
    if (font)
      break;
    std::cerr << "CreateFont:" << "" << "\n";
  }

  if (font) {
    // should return metrics?
    if (fm) {
      fm->ascent = font->ascent();
      fm->descent = font->descent();
      fm->height = font->height();
      fm->x_height = font->height();
    }
  }
  else {
    std::cerr << "ERROR couldnt find '" << faceName << "'\n";
  }

  //printf("%p\n", font);

  return font;
}

void sdl_doc_container::delete_font(litehtml::uint_ptr hFont)
{
  ftfont * font = (ftfont *)hFont;
  delete font;
}

int sdl_doc_container::text_width(const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont)
{
  ftfont * font = (ftfont *)hFont;
  if (font) {
    int width = font->text_width(text);
    //std::cout << "#text_width(" << strlen(text) << ")'" << (char *)text << "' = " << width << "\n";
    return width;
  }
  else {
    if (font)
      std::cerr << "text_width(" << text << ") failed: " << "" << "\n";
  }
  return 0;
}

void sdl_doc_container::draw_text(litehtml::uint_ptr hdc,
                                  const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont,
                                  litehtml::web_color color,
                                  const litehtml::position & pos)
{
  //ftfont * font = (ftfont *)hFont;
  std::cout << "#draw_text '" << text << "' c: " << color << "\n";
#if 0
  if (SDL_Surface * surface = TTF_RenderUTF8_Solid(font, text, fg)) {
    SDL_Texture * texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_Rect dst = {pos.x, pos.y - (int)(pos.height * 0.5), surface->w, surface->h};
    SDL_RenderCopy(_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
  }
  else {
    std::cerr << "TTF_RenderUTF8_Solid(" << text << ") failed: " << TTF_GetError() << "\n";
  }
#endif
}

int sdl_doc_container::pt_to_px(int pt)
{
  std::cout << "#pt_to_px " << pt << "\n";
  return pt;
}

int sdl_doc_container::get_default_font_size() const
{
  return 16;
}

const litehtml::tchar_t * sdl_doc_container::get_default_font_name() const
{
  return _t("sans-serif");
}

void sdl_doc_container::draw_list_marker(litehtml::uint_ptr hdc,
                                         const litehtml::list_marker & marker)
{
  if (!marker.image.empty()) {
    litehtml::tstring url;
    make_url(marker.image.c_str(), marker.baseurl, url);
    if (_images.count(url)) {
      SDL_Surface * surface = _images.at(url).first;
      SDL_Texture * texture = _images.at(url).second;
      SDL_Rect dst = conv(marker.pos);
      SDL_Rect src = { 0, 0, surface->w, surface->h };
      SDL_RenderCopy(_renderer, texture, &src, &dst);
      std::cout << "Drawing... " << url << " " << surface->w << "," << surface->h << "\n";
    }
      
  }
  else {
    switch(marker.marker_type) {
    case litehtml::list_style_type_circle:
      break;
    case litehtml::list_style_type_disc:
      break;
    case litehtml::list_style_type_square:
      break;
    default:
      /*do nothing*/
      break;
    }
    SDL_Rect rect = conv(marker.pos);
    setrendererdrawcolor(marker.color);
    SDL_RenderFillRect(_renderer, &rect);
  }
  std::cout << "#draw_list_marker" << "\n";
}

void sdl_doc_container::load_image(const litehtml::tchar_t * src,
                                   const litehtml::tchar_t * baseurl,
                                   bool redraw_on_ready)
{
  litehtml::tstring url;
  make_url(src, baseurl, url);
  //std::cout << "#load_image '" << url << "' | '" << src << "'\n";

  SDL_Surface * surface = STBIMG_Load(url.c_str());
  SDL_Texture * texture = SDL_CreateTextureFromSurface(_renderer, surface);
  if (surface == nullptr || texture == nullptr) {
    printf("ERROR: Couldn't load %s, reason: %s\n", url.c_str(), SDL_GetError());
    exit(-1);
  }
  else {
    _images[url] = {surface, texture};
  }
}

void sdl_doc_container::get_image_size(const litehtml::tchar_t * src,
                                       const litehtml::tchar_t * baseurl,
                                       litehtml::size & sz)
{
  litehtml::tstring url;
  make_url(src, baseurl, url);

  if (_images.count(url)) {
    sz.width = _images.at(url).first->w;
    sz.height = _images.at(url).first->h;
  }
  std::cout << "#get_image_size '" << url
            << "' = " << sz.width << "x" << sz.height << "\n";
}

void sdl_doc_container::draw_background(litehtml::uint_ptr hdc,
                                        const litehtml::background_paint & bg)
{
  std::cout << "#draw_background " << "\n";
  if (bg.image.empty()) {
    SDL_Rect rect = conv(bg.clip_box);
    setrendererdrawcolor(bg.color);
    SDL_RenderFillRect(_renderer, &rect);
  }
  else {
    // Drawing image
    litehtml::tstring url;
    make_url(bg.image.c_str(), (bg.baseurl.size() ? bg.baseurl.c_str() : nullptr), url);

    if (_images.count(url)) {
      SDL_Surface * surface = _images.at(url).first;
      SDL_Texture * texture = _images.at(url).second;
      SDL_Rect dst = { bg.clip_box.x, bg.clip_box.y, bg.image_size.width, bg.image_size.height };
      SDL_Rect src = { 0, 0, surface->w, surface->h };
      SDL_RenderCopy(_renderer, texture, &src, &dst);
      std::cout << "Drawing... " << url << " " << src << "->" << dst << "\n";
      switch(bg.repeat) {
      case litehtml::background_repeat_no_repeat:
        //bg.position_x, bg.position_y, bgbmp->getWidth(), bgbmp->getHeight()
        break;
      case litehtml::background_repeat_repeat_x:
        //bg.clip_box.left(), bg.position_y, bg.clip_box.width, bgbmp->getHeight()
        break;
      case litehtml::background_repeat_repeat_y:
        //bg.position_x, bg.clip_box.top(), bgbmp->getWidth(), bg.clip_box.height
        break;
      case litehtml::background_repeat_repeat:
        //bg.clip_box.left(), bg.clip_box.top(), bg.clip_box.width, bg.clip_box.height
        break;
      }
    }
    else {
      std::cerr << "unable to find image '" << url << "'\n";
    }
  }
}

void sdl_doc_container::draw_borders(litehtml::uint_ptr hdc,
                                     const litehtml::borders & borders,
                                     const litehtml::position & draw_pos,
                                     bool root)
{
  std::cout << "#draw_borders " << draw_pos.x << "," << draw_pos.y
            << " " << draw_pos.width << "x" << draw_pos.height
            << " c: " << borders.top.color << "\n";
  auto draw_rect = [&](SDL_Rect & rect, const litehtml::border & bo) {
    if (!rect.w || !rect.h || bo.style == litehtml::border_style_hidden)
      return;
    setrendererdrawcolor(bo.color);
    SDL_RenderDrawRect(_renderer, &rect);
    //SDL_RenderFillRect(_renderer, &rect);
    std::cout << " " << rect << " w=" << bo.width << "\n";
  };
  SDL_Rect rect;
  // top
  rect = { draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height }; // borders.top.width
  draw_rect(rect, borders.top);
  return;
  // bottom
  rect = { draw_pos.x, draw_pos.y + draw_pos.height - borders.bottom.width, draw_pos.width, borders.bottom.width };
  draw_rect(rect, borders.bottom);
  // left
  rect = { draw_pos.x, draw_pos.y, borders.left.width, draw_pos.height };
  draw_rect(rect, borders.left);
  // right
  rect = { draw_pos.x + draw_pos.height - borders.right.width, draw_pos.y, borders.right.width, draw_pos.height };
  draw_rect(rect, borders.right);
}

void sdl_doc_container::set_caption(const litehtml::tchar_t * caption)
{
  std::cout << "#set_caption " << caption << "\n";
}

void sdl_doc_container::set_base_url(const litehtml::tchar_t * base_url)
{
  std::cout << "#set_base_url " << base_url << "\n";
  _base_url = base_url;
}

void sdl_doc_container::link(const std::shared_ptr < litehtml::document > & doc,
                             const litehtml::element::ptr & el)
{
  if (el) {
    litehtml::tstring text;
    el->get_text(text);
    //std::cout << "#link " << text << "\n";
  }
}

void sdl_doc_container::on_anchor_click(const litehtml::tchar_t * url,
                                        const litehtml::element::ptr & el)
{
  std::cout << "#on_anchor_click " << url << "\n";
}

void sdl_doc_container::set_cursor(const litehtml::tchar_t * cursor)
{
  //std::cout << "#set_cursor " << cursor << "\n";
}

void sdl_doc_container::transform_text(litehtml::tstring & text,
                                       litehtml::text_transform tt)
{
  std::cout << "#transform_text " << text << "\n";
}

void sdl_doc_container::import_css(litehtml::tstring & text,
                                   const litehtml::tstring & url,
                                   litehtml::tstring & baseurl)
{
  litehtml::tstring full_url;
  //make_url(url, baseurl, full_url);

  std::cout << "#import_css " << text << " | " << url << " | " << baseurl << "\n";
}

void sdl_doc_container::set_clip(const litehtml::position & pos,
                                 const litehtml::border_radiuses & bdr_radius,
                                 bool valid_x,
                                 bool valid_y)
{
  std::cout << "#set_clip " << "pos" << "\n";
}

void sdl_doc_container::del_clip()
{
  std::cout << "#del_clip\n";
}

void sdl_doc_container::get_client_rect(litehtml::position & client) const
{
  if (_renderer) {
    client.x = 0;
    client.y = 0;
    SDL_Rect rect;
    SDL_RenderGetViewport(_renderer, &rect);
    //int w, h;
    //SDL_RenderGetLogicalSize(_renderer, &w, &h);
    client.width = rect.w;
    client.height = rect.h;
  }
  //std::cout << "#get_client_rect " << client.x << "," << client.y
  //          << "  " << client.width << "x" << client.height << "\n";
}

std::shared_ptr < litehtml::element >
sdl_doc_container::create_element(const litehtml::tchar_t * tag_name,
                                  const litehtml::string_map & attributes,
                                  const std::shared_ptr < litehtml::document > & doc)
{
  //std::cout << "#create_element:\n";
  //for (auto & el : attributes)
  //  std::cout << "    " << el.first << " -> " << el.second << "\n";
  return std::shared_ptr < litehtml::element >();
}

void sdl_doc_container::get_media_features(litehtml::media_features & media) const
{
  litehtml::position client;
  get_client_rect(client);
  media.type = litehtml::media_type_screen;
  media.width = client.width;
  media.height = client.height;
  media.device_width = 512;
  media.device_height = 512;
  media.color = 8;
  media.monochrome = 0;
  media.color_index = 256;
  media.resolution = 96;
}

void sdl_doc_container::get_language(litehtml::tstring & language,
                                     litehtml::tstring & culture) const
{
  language = _t("en");
  culture = _t("");
}

litehtml::tstring sdl_doc_container::resolve_color(const litehtml::tstring & color) const
{
  std::cout << "#resolve_color " << color << "\n";
  return litehtml::tstring();
}
