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
#include <fstream>
#include "sdl-container.h"
#include "thirdparty/butf8.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#define STBI_NO_LINEAR
#define STBI_NO_HDR
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

bool file_to_string(const litehtml::tstring & filename, litehtml::tstring & dst)
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

SDL_Rect conv(const litehtml::position & pos)
{
  SDL_Rect dst;
  dst.x = pos.x;
  dst.y = pos.y;
  dst.w = pos.width;
  dst.h = pos.height;
  return dst;
}

class ftfont {

public:
  static const int FONT_STYLE_ITALIC = 0x01;
  static const int FONT_STYLE_STRIKETHROUGH = 0x02;
  static const int FONT_STYLE_UNDERLINE = 0x04;

  static ftfont * CreateFont(const char * file, int size, int weight, int style)
  {
    // load the font desc into memory
    FT_Face face;
    FT_Error err = FT_New_Face(g_freetype, file, 0, &face);
    if (!err) {
      err = FT_Set_Char_Size(face, size * 64, size * 64, 72, 0);
      if (err) {
        std::cerr << "FT_Set_Char_Size ERR: " << err << "\n";
        FT_Done_Face(face);
        return nullptr;
      }

      if (0) {
        std::cout << "loaded " << file << ":" << size
                  << " style: " << face->style_name
                  << " bbox: [x:" << face->bbox.xMin << "-" << face->bbox.xMax
                  << " y:" << face->bbox.yMin << "-" << face->bbox.yMax << "]\n"
                  << " num_glyphs: " << face->num_glyphs
                  << " height: " << face->height
                  << " ascent: " << face->ascender
                  << " descent: " << face->descender
                  << " units_per_EM: " << face->units_per_EM
                  << " num_fixed_sizes: " << face->num_fixed_sizes << "\n";
      }
    }
    else {
      std::cerr << "FT_New_Face ERR: " << err << "\n";
      return nullptr;
    }

    ftfont * font = new ftfont();
    font->_face = face;
    font->_style = style;
    if (0) {
      std::cout << " size  :     " << font->size() << "\n"
                << " ascent:     " << font->ascent() << "\n"
                << " descent:    " << font->descent() << "\n"
                << " lineheight: " << font->lineheight() << "\n"
                << " height:     " << font->height() << "\n";
    }
    return font;
  }

  ftfont() {
  }

  ~ftfont() {
    if (_texture)
      SDL_DestroyTexture(_texture);
    FT_Done_Face(_face);
  }

  int size() { return _face->size->metrics.height / 64; }
  int ascent() { return _face->size->metrics.ascender / 64; }
  int descent() { return _face->size->metrics.descender / 64; }
  int lineheight() { return _face->size->metrics.height / 64; }
  int height() { return _face->size->metrics.height / 64; }

  int text_width(const litehtml::tchar_t * text)
  {
    uint32_t ch;
    int width = 0;
    int er = 0;
    //static void * cur = (void *)text;
    //for (cur = utf8_decode(cur, &ch, &er); ch && !er; cur = utf8_decode(cur, &ch, &er)) {
    while ((ch = getUTF8Next(text))) {
      FT_Error err = 0;
      FT_UInt gindex = FT_Get_Char_Index(_face, ch);
      if (0 != gindex && !(err = FT_Load_Glyph(_face, gindex, FT_LOAD_BITMAP_METRICS_ONLY))) {
        FT_GlyphSlot slot = _face->glyph;
        width += slot->metrics.horiAdvance / 64;
      }
      else {
        std::cerr << "text_width err: " << err << " ch=" << ch << " gi=" << gindex << " e=" << er << "\n";
      }
    }
    return width;
  }

  void draw_text(SDL_Renderer * renderer,
                 const litehtml::tchar_t * text,
                 litehtml::web_color color,
                 const litehtml::position & pos)
  {
    uint32_t ch;

    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(_texture, color.red, color.green, color.blue);
    SDL_SetTextureAlphaMod(_texture, color.alpha);

    SDL_Rect dst = conv(pos);
    int xpos = dst.x;
    int ypos = dst.y + dst.h + descent();
    while ((ch = getUTF8Next(text))) {
      glyph_info glyph;
      if (get_glyph_info(renderer, ch, glyph)) {
        if (glyph.rect.w && glyph.rect.h) {
          dst.x = xpos + glyph.xoff;
          dst.y = ypos + glyph.yoff;
          dst.w = glyph.rect.w;
          dst.h = glyph.rect.h;
          SDL_RenderCopy(renderer, _texture, &glyph.rect, &dst);
        }
        xpos += glyph.xadv;
      }
    }

    if (0) {
      SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, 64);
      SDL_Rect rect = conv(pos);
      SDL_RenderFillRect(renderer, &rect);
    }
  }

private:
  static const int TSIZE = 1024;

  struct glyph_info {
    SDL_Rect rect;
    int xoff;
    int yoff;
    int xadv;
  };

  bool get_glyph_info(SDL_Renderer * renderer, uint32_t ch, glyph_info & glyph)
  {
    // Check if we already have a glyph rendered
    auto it = _glyphs.find(ch);
    if (it != _glyphs.end()) {
      glyph = it->second;
      return true;
    }

    if (!_texture) {
      Uint32 format = SDL_PIXELFORMAT_RGBA8888;
      if (!(_texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STATIC, TSIZE, TSIZE))) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        return false;
      }
    }

    FT_Error err = 0;
    if (!(err = FT_Load_Char(_face, ch, FT_LOAD_RENDER))) {
      FT_GlyphSlot slot = _face->glyph;
      //std::cout << "cp:" << ch << " ha:" << slot->metrics.horiAdvance << "\n";

      if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        std::cerr << "Unknown glyph format: " << (int)slot->bitmap.pixel_mode << " for ch " << ch << "\n";
        return false;
      }

      // find space in the texture for the new glyph
      if (!get_empty_rect(slot->bitmap.width, slot->bitmap.rows, glyph.rect))
        return false;

      // store the glyph info
      glyph.xadv = slot->advance.x / 64;
      glyph.xoff = slot->bitmap_left;
      glyph.yoff = -slot->bitmap_top; //-(slot->metrics.horiBearingY / 64); //slot->metrics.height 
      //glyph.yoff = 0;
      _glyphs[ch] = glyph;

      // if it's an empty glyph but valid, ie space ' ', just return
      size_t npix = slot->bitmap.width * slot->bitmap.rows;
      if (!npix)
        return true;

      // expand the glyph bitmap to RGBA8888
      uint32_t bytes[npix];
      for (unsigned rr = 0; rr < slot->bitmap.rows; rr++)
        for (unsigned cc = 0; cc < slot->bitmap.width; cc++) {
          uint8_t grey = slot->bitmap.buffer[rr * slot->bitmap.pitch + cc];
          bytes[rr * slot->bitmap.pitch + cc] = 0x01010101 * grey;
        }

      // update the texture with the glypy
      if (SDL_UpdateTexture(_texture, &glyph.rect, bytes, slot->bitmap.width * 4)) {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << "\n";
        return false;
      }

      return true;
    }
    else {
      std::cerr << "text_width err: " << err << " gi=" << ch << " e=" << err << "\n";
      return false;
    }
  }

  // manage the texture allocation
  bool get_empty_rect(int w, int h, SDL_Rect & rect)
  {
    if (_col_end + w > TSIZE) {
      // new char row
      _col_end = 0;
      _row_bottom = _row_top + 1;
    }

    if (_row_bottom + h >= TSIZE) {
      std::cerr << "Glyph texture full\n";
      return false;
    }

    // the allocated rect
    rect.x = _col_end;
    rect.y = _row_bottom;
    rect.w = w;
    rect.h = h;

    // update empty position
    _row_top = std::max(_row_top, _row_bottom + h);
    _col_end += w;

    //std::cout << "allocating " << rect << "\n";
    return true;
  }

  FT_Face _face;
  int _style = 0;
  std::map<uint32_t, glyph_info> _glyphs;
  SDL_Texture * _texture = nullptr;
  uint32_t _row_bottom = 0;
  uint32_t _row_top = 0;
  uint32_t _col_end = 0;
};

sdl_doc_container::sdl_doc_container()
{
}

sdl_doc_container::~sdl_doc_container()
{
  for (auto & im : _images) {
    SDL_FreeSurface(im.second.first);
    SDL_DestroyTexture(im.second.second);
  }
}

void sdl_doc_container::set_renderer(SDL_Renderer * renderer)
{
  if (_renderer != renderer) {
    // flush any context associated with old renderer
  }

  _renderer = renderer;
  if (SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND))
    std::cerr << "SetRenderDrawBlendMode failed\n";
}

void sdl_doc_container::set_renderer_draw_color(const litehtml::web_color & color)
{
  if (!_renderer)
    return;
  SDL_SetRenderDrawColor(_renderer, color.red, color.green, color.blue, color.alpha);
}

void sdl_doc_container::make_url(const litehtml::tchar_t * src,
                                 const litehtml::tchar_t * baseurl,
                                 litehtml::tstring & url)
{
  //std::cout << "#make_url '" << src << "' | '" << (baseurl ? baseurl : "0") << "' | " << _base_url << "'\n";

  if (!baseurl || !baseurl[0]) {
    if (!_base_url.empty())
      url = _base_url + "/" + src;
    else
      url = src;
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

  for (litehtml::string_vector::iterator i = fonts.begin(); i != fonts.end(); i++) {
    //std::cout << " trying '" << i->c_str() << "'\n";
    font = ftfont::CreateFont((*i + ".ttf").c_str(), size, weight, style);
    if (!font)
      continue;

    if (fm) {
      fm->ascent = font->ascent();
      fm->descent = -font->descent();
      fm->height = font->height();
      fm->x_height = font->height();
    }
  }

  if (!font) {
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
    std::cerr << "#text_width(" << text << ") failed: " << "no font" << "\n";
  }
  return 0;
}

void sdl_doc_container::draw_text(litehtml::uint_ptr hdc,
                                  const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont,
                                  litehtml::web_color color,
                                  const litehtml::position & pos)
{
  if (ftfont * font = (ftfont *)hFont) {
    SDL_Rect r = conv(pos);
    //std::cout << "#draw_text(" << font->size() << ") '" << text << "' c: " << color << " @ " << r << "\n";
    litehtml::position p2 = pos;
    font->draw_text(_renderer, text, color, p2);
  }
  else {
    std::cerr << "draw_text(" << text << ") failed: " << "" << "\n";
  }
}

int sdl_doc_container::pt_to_px(int pt)
{
  double dpi = 96;
  int px = (int) ((double)pt * dpi / 72.0);
  //std::cout << "#pt_to_px " << pt << " -> " << px << "\n";
  return px;
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
    set_renderer_draw_color(marker.color);
    SDL_RenderFillRect(_renderer, &rect);
  }
  //std::cout << "#draw_list_marker" << "\n";
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
  else {
    sz.width = 0;
    sz.height = 0;
    if (strlen(src))
      std::cerr << "#get_image_size unable to find image " << url << ":'" << src << "'\n";
  }
  //std::cout << "#get_image_size '" << url
  // << "' = " << sz.width << "x" << sz.height << "\n";
}

void sdl_doc_container::draw_background(litehtml::uint_ptr hdc,
                                        const litehtml::background_paint & bg)
{
  SDL_Rect clip_box = conv(bg.clip_box);
  SDL_Rect border_box = conv(bg.border_box);
  SDL_Rect origin_box = conv(bg.origin_box);
  std::cout << "#draw_background " << clip_box << "|" << border_box << "|" << origin_box << "\n";

  // border 
  set_renderer_draw_color(bg.color);
  SDL_RenderFillRect(_renderer, &clip_box);

  set_renderer_draw_color(bg.color);
  SDL_RenderFillRect(_renderer, &clip_box);

  // any image
  if (bg.image.empty()) {
    set_renderer_draw_color(bg.color);
    SDL_RenderFillRect(_renderer, &clip_box);
  }
  else {
    // Drawing image
    litehtml::tstring url;
    make_url(bg.image.c_str(), bg.baseurl.c_str(), url);

    if (_images.count(url)) {
      SDL_Surface * surface = _images.at(url).first;
      SDL_Texture * texture = _images.at(url).second;
      SDL_Rect dst;
      SDL_Rect src = { 0, 0, surface->w, surface->h };
      switch (bg.repeat) {
      case litehtml::background_repeat_no_repeat:
         dst = { clip_box.x, clip_box.y, bg.image_size.width, bg.image_size.height };
        break;
      case litehtml::background_repeat_repeat_x:
         dst = { clip_box.x, bg.position_y, clip_box.w, surface->h };
        break;
      case litehtml::background_repeat_repeat_y:
         dst = { bg.position_x, clip_box.y, surface->w, clip_box.h };
        break;
      case litehtml::background_repeat_repeat:
         dst = { clip_box.x, clip_box.y, clip_box.w, clip_box.h };
        break;
      default:
        std::cerr << "unknown background repeat\n";
        return;
      }
      SDL_RenderCopy(_renderer, texture, &src, &dst);
      std::cout << "Drawing... " << url << " " << src << "->" << dst << "\n";
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
  if (0) {
    std::cout << "#draw_borders " << draw_pos.x << "," << draw_pos.y
              << " " << draw_pos.width << "x" << draw_pos.height
              << " c: " << borders.top.color << "\n";
  }

  // draw the border rect
  auto draw_rect = [&](SDL_Rect & rect, const litehtml::border & bo) {
    if (!rect.w || !rect.h || bo.style == litehtml::border_style_hidden)
      return;
    set_renderer_draw_color(bo.color);
    SDL_RenderDrawRect(_renderer, &rect);
    //SDL_RenderFillRect(_renderer, &rect);
    //std::cout << " " << rect << " w=" << bo.width << "\n";
  };

  SDL_Rect rect;
  // all
  if (0) {
    rect = conv(draw_pos);
    draw_rect(rect, borders.top);
    return;
  }
  // top
  rect = { draw_pos.x, draw_pos.y, draw_pos.width, borders.top.width };
  draw_rect(rect, borders.top);
  // bottom
  rect = { draw_pos.x, draw_pos.y + draw_pos.height - borders.bottom.width, draw_pos.width, borders.bottom.width };
  draw_rect(rect, borders.bottom);
  // left
  rect = { draw_pos.x, draw_pos.y, borders.left.width, draw_pos.height };
  draw_rect(rect, borders.left);
  // right
  rect = { draw_pos.x + draw_pos.width - borders.right.width, draw_pos.y, borders.right.width, draw_pos.height };
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
  litehtml::tstring css_url;
  make_url(url.c_str(), baseurl.c_str(), css_url);
  if (file_to_string(css_url, text)) {
    baseurl = css_url;
  }
  else {
    std::cout << "#import_css failed " << css_url << "\n";
  }
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
    client.x = 0;//_x;
    client.y = 0;//_y;
    SDL_Rect rect;
    SDL_RenderGetViewport(_renderer, &rect);
    //SDL_RenderGetLogicalSize(_renderer, &rect.w, &rect.h);
    //SDL_GetRendererOutputSize(_renderer, &rect.w, &rect.h);
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
  media.device_width = client.width;
  media.device_height = client.height;
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
