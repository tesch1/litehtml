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

#include "sdl-container.h"

sdl_doc_container::sdl_doc_container()
{
}

litehtml::uint_ptr
sdl_doc_container::create_font(const litehtml::tchar_t * faceName,
                               int size,
                               int weight,
                               litehtml::font_style italic,
                               unsigned int decoration,
                               litehtml::font_metrics * fm)
{
}

void sdl_doc_container::delete_font(litehtml::uint_ptr hFont)
{
}

int sdl_doc_container::text_width(const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont)
{
}

void sdl_doc_container::draw_text(litehtml::uint_ptr hdc,
                                  const litehtml::tchar_t * text,
                                  litehtml::uint_ptr hFont,
                                  litehtml::web_color color,
                                  const litehtml::position & pos)
{
}

int sdl_doc_container::pt_to_px(int pt)
{
}

int sdl_doc_container::get_default_font_size() const
{
}

const litehtml::tchar_t * sdl_doc_container::get_default_font_name() const
{
  return "Roboto";
}

void sdl_doc_container::draw_list_marker(litehtml::uint_ptr hdc,
                                         const litehtml::list_marker & marker)
{
}

void sdl_doc_container::load_image(const litehtml::tchar_t * src,
                                   const litehtml::tchar_t * baseurl,
                                   bool redraw_on_ready)
{
}

void sdl_doc_container::get_image_size(const litehtml::tchar_t * src,
                                       const litehtml::tchar_t * baseurl,
                                       litehtml::size & sz)
{
}

void sdl_doc_container::draw_background(litehtml::uint_ptr hdc,
                                        const litehtml::background_paint & bg)
{
}

void sdl_doc_container::draw_borders(litehtml::uint_ptr hdc,
                                     const litehtml::borders & borders,
                                     const litehtml::position & draw_pos,
                                     bool root)
{
}

void sdl_doc_container::set_caption(const litehtml::tchar_t * caption)
{
}

void sdl_doc_container::set_base_url(const litehtml::tchar_t * base_url)
{
}

void sdl_doc_container::link(const std::shared_ptr < litehtml::document > & doc,
                             const litehtml::element::ptr & el)
{
}

void sdl_doc_container::on_anchor_click(const litehtml::tchar_t * url,
                                        const litehtml::element::ptr & el)
{
}

void sdl_doc_container::set_cursor(const litehtml::tchar_t * cursor)
{
}

void sdl_doc_container::transform_text(litehtml::tstring & text,
                                       litehtml::text_transform tt)
{
}

void sdl_doc_container::import_css(litehtml::tstring & text,
                                   const litehtml::tstring & url,
                                   litehtml::tstring & baseurl)
{
}

void sdl_doc_container::set_clip(const litehtml::position & pos,
                                 const litehtml::border_radiuses & bdr_radius,
                                 bool valid_x,
                                 bool valid_y)
{
}

void sdl_doc_container::del_clip()
{
}

void sdl_doc_container::get_client_rect(litehtml::position & client) const
{
}

std::shared_ptr < litehtml::element >
sdl_doc_container::create_element(const litehtml::tchar_t * tag_name,
                                  const litehtml::string_map & attributes,
                                  const std::shared_ptr < litehtml::document > & doc)
{
  return std::shared_ptr < litehtml::element >();
}

void sdl_doc_container::get_media_features(litehtml::media_features & media) const
{
}

void sdl_doc_container::get_language(litehtml::tstring & language,
                                     litehtml::tstring & culture) const
{
}

litehtml::tstring sdl_doc_container::resolve_color(const litehtml::tstring & color) const
{
  return litehtml::tstring();
}
