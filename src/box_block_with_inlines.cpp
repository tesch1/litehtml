#include "html.h"
#include "box_block_with_inlines.h"
#include "box_inline.h"
#include "document.h"

void litehtml::box_model::box_block_with_inlines::add_box(const box_base::ptr& box)
{
	if (box->get_type() == box_type_inline)
	{
		box->fetch_by_type(box_type_element, m_element_boxes);
		push_child_box(box);
	}
	else
	{
		m_element_boxes.push_back(box);
		bool add_anonymous_box = true;
		if (!m_children.empty())
		{
			if (m_children.back()->is_anonymous())
			{
				m_children.back()->add_box(box);
				add_anonymous_box = false;
			}
		}
		if (add_anonymous_box)
		{
			box_base::ptr anon_box = std::make_shared<box_inline>(nullptr, std::shared_ptr<document>(m_document));
			push_child_box(anon_box);
			anon_box->add_box(box);
		}
	}
}

int litehtml::box_model::box_block_with_inlines::render_children(int x, int y, int max_width, int ret_width, bool second_pass)
{
	int lineheight = line_height();
	font_metrics fm;
	get_font(&fm);

	m_lines.clear();
	box_line* ln = add_line(max_width, lineheight, fm);

	bool add = false;
	for (auto& box : m_element_boxes)
	{
		add = true;
		if (box->is_break())
		{
			// add new line on break
			ln = add_line(max_width, lineheight, fm);
			add = false;
		}
		else if (box->is_white_space())
		{
			// don't add white space at the beginning of line and if the last elemet was the white space
			if (ln->elements.empty() || !ln->elements.empty() && ln->elements.back()->is_white_space())
			{
				add = false;
			}
		}
		if (add)
		{
			// render box to get the sizes
			int rw = box->render(0, 0, max_width, second_pass);
			if (rw > ret_width)
			{
				ret_width = rw;
			}
			if (box->width() + ln->pos.width > max_width)
			{
				// we need new line here
				ln = add_line(max_width, lineheight, fm);
				if (box->is_white_space())
				{
					// don't add white space at the beginning of line
					add = false;
				}
			}
			if (add)
			{
				box->move_to(ln->pos.width + box->content_margins_left(), 0);
				box->update_element();

				ln->add_box(box);
			}
		}
	}
	m_lines.back().finish(lineheight, fm, m_text_align);
	m_pos.height = ln->pos.bottom();
	return ret_width;
}

litehtml::box_model::box_line* litehtml::box_model::box_block_with_inlines::add_line(int max_width, int line_height, font_metrics& font)
{
	int new_top = 0;
	box_line* ret = nullptr;
	if (!m_lines.empty())
	{
		new_top = m_lines.back().pos.bottom();
		m_lines.back().finish(line_height, font, m_text_align);
	}
	// TODO: This code must be changed to support floating boxes
	m_lines.emplace_back(0, max_width, new_top, font.base_line(), line_height);
	
	return &m_lines.back();
}

void litehtml::box_model::box_block_with_inlines::draw_children(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex)
{
	if (flag == draw_inlines)
	{
		position pos = m_pos;
		pos.x += x;
		pos.y += y;

		document::ptr doc(m_document);

		if (get_overflow() > overflow_visible)
		{
			position border_box = pos;
			border_box += m_padding;
			border_box += m_borders;

			border_radiuses bdr_radius = m_css_borders.radius.calc_percents(border_box.width, border_box.height);

			bdr_radius -= m_borders;
			bdr_radius -= m_padding;

			doc->container()->set_clip(pos, bdr_radius, true, true);
		}

		box_base::ptr el;
		for (auto& line : m_lines)
		{
			for (auto& box : line.elements)
			{
				box->draw(hdc, pos.x + line.pos.x, pos.y + line.pos.y, clip);
				if (box->get_element_display() == display_inline_block)
				{
					box->draw_stacking_context(hdc, pos.x + line.pos.x, pos.y + line.pos.y, clip, false);
				}
			}
		}

		if (get_overflow() > overflow_visible)
		{
			doc->container()->del_clip();
		}
	}
}

void litehtml::box_model::box_block_with_inlines::init_from_element()
{
	box_block::init_from_element();
	m_text_align = (text_align)value_index(get_style_property(_t("text-align"), true, _t("left")), text_align_strings, text_align_left);
}

int litehtml::box_model::box_block_with_inlines::calculate_base_line() const
{
	if (!m_lines.empty())
	{
		return m_pos.height + content_margins_bottom() - (m_lines.back().pos.bottom() - m_lines.back().base_line);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

void litehtml::box_model::box_line::add_box(const box_base::ptr& box)
{
	// find maximum baseline and line height
	// for elements with style "vertical-align=baseline"
	if (box->get_vertical_align() == va_baseline)
	{
		base_line = std::max(base_line, box->calculate_base_line());
		line_height = std::max(line_height, box->line_height());
		pos.width += box->width();
		pos.height = std::max(pos.height, box->height());
	}

	elements.push_back(box);
}

void litehtml::box_model::box_line::finish(int lineheight, font_metrics& font, text_align align)
{
	// remove last white space
	if (!elements.empty() && elements.back()->is_white_space())
	{
		pos.width -= elements.back()->width();
		elements.pop_back();
	}

	int add_x = 0;
	switch (align)
	{
	case text_align_right:
		if (pos.width < (box_right - box_left))
		{
			add_x = (box_right - box_left) - pos.width;
		}
		break;
	case text_align_center:
		if (pos.width < (box_right - box_left))
		{
			add_x = ((box_right - box_left) - pos.width) / 2;
		}
		break;
	default:
		add_x = 0;
		break;
	}

	for (const auto& el : elements)
	{
		el->move(add_x, 0);
	}

	if (pos.height)
	{
		base_line += (lineheight - pos.height) / 2;
	}

	pos.height = lineheight;

	int y1 = 0;
	int y2 = pos.height;

	for (const auto& el : elements)
	{
		if (el->get_element_display() == display_inline_text)
		{
			font_metrics fm;
			el->get_font(&fm);
			el->move_to(el->content_left(), pos.height - base_line - fm.ascent);
		}
		else
		{
			switch (el->get_vertical_align())
			{
			case va_top:
				el->move_to(el->content_left(),
					y1 + el->content_margins_top());
				break;
			case va_text_top:
				el->move_to(el->content_left(),
					pos.height - base_line - font.ascent + el->content_margins_top());
				break;
			case va_middle:
				el->move_to(el->content_left(),
					pos.height - base_line - font.x_height / 2 - el->height() / 2 + el->content_margins_top());
				break;
			case va_bottom:
				el->move_to(el->content_left(),
					y2 - el->height() + el->content_margins_top());
				break;
			case va_text_bottom:
				el->move_to(el->content_left(),
					pos.height - base_line + font.descent - el->height() + el->content_margins_top());
				break;
			default:
				el->move_to(el->content_left(),
					pos.height - base_line - el->height() + el->calculate_base_line() + el->content_margins_top());
				break;
			}
			y1 = std::min(y1, el->top());
			y2 = std::max(y2, el->bottom());
		}
	}

	for (const auto& el : elements)
	{
		int pos_y = el->content_top() - y1;
		if (el->get_element_display() != display_inline_text)
		{
			switch (el->get_vertical_align())
			{
			case va_top:
				pos_y = el->content_margins_top();
				break;
			case va_bottom:
				pos_y = (y2 - y1) - el->height() + el->content_margins_top();
				break;
			case va_baseline:
				//TODO: process vertical align "baseline"
				break;
			case va_middle:
				//TODO: process vertical align "middle"
				break;
			case va_sub:
				//TODO: process vertical align "sub"
				break;
			case va_super:
				//TODO: process vertical align "super"
				break;
			case va_text_bottom:
				//TODO: process vertical align "text-bottom"
				break;
			case va_text_top:
				//TODO: process vertical align "text-top"
				break;
			}
		}
		el->move_to(el->content_left(), pos_y);
		el->apply_relative_shift(box_right - box_left);
		el->update_element();
	}
	pos.height = y2 - y1;
	base_line = (base_line - y1) - (pos.height - lineheight);
}

