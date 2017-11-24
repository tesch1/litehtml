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
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

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

litehtml::uint_ptr
sdl_doc_container::create_font(const litehtml::tchar_t * faceName,
                               int size,
                               int weight,
                               litehtml::font_style italic,
                               unsigned int decoration,
                               litehtml::font_metrics * fm)
{
  std::cout << "#create_font " << faceName << "\n";
  litehtml::string_vector fonts;
  litehtml::split_string(faceName, fonts, ",");
  litehtml::trim(fonts[0]);

  bool found = false;
  for (litehtml::string_vector::iterator i = fonts.begin(); i != fonts.end(); i++) {
    if (FcPatternAddString(pattern, FC_FAMILY, (unsigned char *) i->c_str())) {
      found = true;
      break;
    }
  }
  if (found) {
    if (italic == litehtml::fontStyleItalic) {
    }
    else {
    }
  }
  if (fm && found) {
    fm->ascent		= (int) ext.ascent;
    fm->descent		= (int) ext.descent;
    fm->height		= (int) (ext.ascent + ext.descent);
    fm->x_height	= (int) tex.height;
    ret->strikeout 	= (decoration & litehtml::font_decoration_linethrough) ? true : false;
    ret->underline	= (decoration & litehtml::font_decoration_underline) ? true : false;
  }

  return 0;
}

void sdl_doc_container::delete_font(litehtml::uint_ptr hFont)
{
  std::cout << "#delete_font\n";
}

int sdl_doc_container::text_width(const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont)
{
  //std::cout << "#text_width " << text << "\n";
  return strlen(text) * 8;
}

void sdl_doc_container::draw_text(litehtml::uint_ptr hdc,
                                  const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont,
                                  litehtml::web_color color,
                                  const litehtml::position & pos)
{
  std::cout << "#draw_text " << text << "\n";
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
    /*
      litehtml::tstring url;
      make_url(marker.image.c_str(), marker.baseurl, url);
      lock_images_cache();
      images_map::iterator img_i = m_images.find(url.c_str());
      if(img_i != m_images.end())
      {
      if(img_i->second)
      {
      draw_txdib((cairo_t*) hdc, img_i->second, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
      }
      }
      unlock_images_cache();
    */
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
  }
  std::cout << "#draw_list_marker" << "\n";
}

void sdl_doc_container::load_image(const litehtml::tchar_t * src,
                                   const litehtml::tchar_t * baseurl,
                                   bool redraw_on_ready)
{
  std::cout << "#load_image '" << (baseurl ? baseurl : "./") << "' | '" << src << "'\n";
}

void sdl_doc_container::get_image_size(const litehtml::tchar_t * src,
                                       const litehtml::tchar_t * baseurl,
                                       litehtml::size & sz)
{
  std::cout << "#get_image_size '" << (baseurl ? baseurl : "./") << "' | '" << src << "'\n";
}

void sdl_doc_container::draw_background(litehtml::uint_ptr hdc,
                                        const litehtml::background_paint & bg)
{
  std::cout << "#draw_background " << "\n";
}

void sdl_doc_container::draw_borders(litehtml::uint_ptr hdc,
                                     const litehtml::borders & borders,
                                     const litehtml::position & draw_pos,
                                     bool root)
{
  std::cout << "#draw_borders " << "\n";
}

void sdl_doc_container::set_caption(const litehtml::tchar_t * caption)
{
  std::cout << "#set_caption " << caption << "\n";
}

void sdl_doc_container::set_base_url(const litehtml::tchar_t * base_url)
{
  std::cout << "#set_base_url " << base_url << "\n";
}

void sdl_doc_container::link(const std::shared_ptr < litehtml::document > & doc,
                             const litehtml::element::ptr & el)
{
  std::cout << "#link " << "" << "\n";
}

void sdl_doc_container::on_anchor_click(const litehtml::tchar_t * url,
                                        const litehtml::element::ptr & el)
{
  std::cout << "#on_anchor_click " << url << "\n";
}

void sdl_doc_container::set_cursor(const litehtml::tchar_t * cursor)
{
  std::cout << "#set_cursor " << cursor << "\n";
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
  std::cout << "#get_client_rect \n";
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
  return litehtml::tstring();
}
