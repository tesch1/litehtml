#include "html.h"
#include "box_block.h"
#include "box_block_with_inlines.h"
#include "document.h"

litehtml::box_model::box_type litehtml::box_model::box_block::get_type() const
{
	if (m_element)
	{
		style_display disp = m_element->get_display();
		if (disp == display_inline_block || disp == display_inline_table)
		{
			return  box_type_element;
		}
	}
	return box_type_block;
}

void litehtml::box_model::box_block::init_from_element()
{
	if (m_element)
	{
		document::ptr doc(m_document);

		m_css_width = m_element->get_raw_css_width(doc);
		m_css_height = m_element->get_raw_css_height(doc);
		m_css_min_width = m_element->get_raw_css_min_width(doc);
		m_css_min_height = m_element->get_raw_css_min_height(doc);
		m_css_max_width = m_element->get_raw_css_max_width(doc);
		m_css_max_height = m_element->get_raw_css_max_height(doc);
		m_css_offsets = m_element->get_raw_css_offset(doc);
		m_css_margins = m_element->get_raw_css_margin(doc);
		m_css_padding = m_element->get_raw_css_padding(doc);
		m_css_borders = m_element->get_raw_css_border(doc);
		m_box_sizing = (box_sizing)value_index(m_element->get_style_property(_t("box-sizing"), false, _t("content-box")), box_sizing_strings, box_sizing_content_box);
		m_float = (element_float)value_index(m_element->get_style_property(_t("float"), false, _t("none")), element_float_strings, float_none);
		m_clear = (element_clear)value_index(m_element->get_style_property(_t("clear"), false, _t("none")), element_clear_strings, clear_none);
		m_overflow = (overflow)value_index(m_element->get_style_property(_t("overflow"), false, _t("visible")), overflow_strings, overflow_visible);
	}
}

litehtml::css_length litehtml::box_model::box_block::get_css_width() const
{
	return m_css_width;
}

litehtml::css_length litehtml::box_model::box_block::get_css_height() const
{
	return m_css_height;
}

void litehtml::box_model::box_block::calc_auto_margins(int parent_width)
{
	if (get_element_position() != element_position_absolute && (get_element_display() == display_block || get_element_display() == display_table))
	{
		if (m_css_margins.left.is_predefined() && m_css_margins.right.is_predefined())
		{
			int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right;
			if (el_width <= parent_width)
			{
				m_margins.left = (parent_width - el_width) / 2;
				m_margins.right = (parent_width - el_width) - m_margins.left;
			}
			else
			{
				m_margins.left = 0;
				m_margins.right = 0;
			}
		}
		else if (m_css_margins.left.is_predefined() && !m_css_margins.right.is_predefined())
		{
			int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right + m_margins.right;
			m_margins.left = parent_width - el_width;
			if (m_margins.left < 0) m_margins.left = 0;
		}
		else if (!m_css_margins.left.is_predefined() && m_css_margins.right.is_predefined())
		{
			int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right + m_margins.left;
			m_margins.right = parent_width - el_width;
			if (m_margins.right < 0) m_margins.right = 0;
		}
	}
}

int litehtml::box_model::box_block::get_floats_height(element_float el_float /*= float_none*/) const
{
	if (is_flow_root())
	{
		int h = 0;

		bool process = false;

		for (const auto& fb : m_floats_left)
		{
			process = false;
			switch (el_float)
			{
			case float_none:
				process = true;
				break;
			case float_left:
				if (fb.clear_floats == clear_left || fb.clear_floats == clear_both)
				{
					process = true;
				}
				break;
			case float_right:
				if (fb.clear_floats == clear_right || fb.clear_floats == clear_both)
				{
					process = true;
				}
				break;
			}
			if (process)
			{
				if (el_float == float_none)
				{
					h = std::max(h, fb.pos.bottom());
				}
				else
				{
					h = std::max(h, fb.pos.top());
				}
			}
		}


		for (const auto fb : m_floats_right)
		{
			process = false;
			switch (el_float)
			{
			case float_none:
				process = true;
				break;
			case float_left:
				if (fb.clear_floats == clear_left || fb.clear_floats == clear_both)
				{
					process = true;
				}
				break;
			case float_right:
				if (fb.clear_floats == clear_right || fb.clear_floats == clear_both)
				{
					process = true;
				}
				break;
			}
			if (process)
			{
				if (el_float == float_none)
				{
					h = std::max(h, fb.pos.bottom());
				}
				else
				{
					h = std::max(h, fb.pos.top());
				}
			}
		}

		return h;
	}
	box_base::ptr el_parent = get_parent();
	if (el_parent)
	{
		int h = el_parent->get_floats_height(el_float);
		return h - m_pos.y;
	}
	return 0;
}

bool litehtml::box_model::box_block::is_flow_root() const
{
	style_display display = get_element_display();
	element_position el_pos = get_element_position();

	if (m_overflow != overflow_visible ||
		m_float != float_none ||
		display == display_table_cell ||
		display == display_table_caption ||
		display == display_inline_block ||
		display == display_inline_table ||
		el_pos != element_position_static ||
		el_pos != element_position_relative)
	{
		return true;
	}
	if (m_element && m_element->is_body())
	{
		return true;
	}
	return false;
}

litehtml::overflow litehtml::box_model::box_block::get_overflow() const
{
	return m_overflow;
}

litehtml::element_float litehtml::box_model::box_block::get_float() const
{
	return m_float;
}

litehtml::element_clear litehtml::box_model::box_block::get_clear() const
{
	return m_clear;
}

void litehtml::box_model::box_block::place_floated_box(const box_base::ptr& floaded_box, int x, int y)
{
	if (is_flow_root())
	{
		floated_box fb;
		fb.pos.x = floaded_box->left() + x;
		fb.pos.y = floaded_box->top() + y;
		fb.pos.width = floaded_box->width();
		fb.pos.height = floaded_box->height();
		fb.float_side = floaded_box->get_float();
		fb.clear_floats = floaded_box->get_clear();
		fb.box = floaded_box;

		if (fb.float_side == float_left)
		{
			if (m_floats_left.empty())
			{
				m_floats_left.push_back(fb);
			}
			else
			{
				bool inserted = false;
				for (floated_box::vector::iterator i = m_floats_left.begin(); i != m_floats_left.end(); i++)
				{
					if (fb.pos.right() > i->pos.right())
					{
						m_floats_left.insert(i, std::move(fb));
						inserted = true;
						break;
					}
				}
				if (!inserted)
				{
					m_floats_left.push_back(std::move(fb));
				}
			}
			m_cahe_line_left.invalidate();
		}
		else if (fb.float_side == float_right)
		{
			if (m_floats_right.empty())
			{
				m_floats_right.push_back(std::move(fb));
			}
			else
			{
				bool inserted = false;
				for (floated_box::vector::iterator i = m_floats_right.begin(); i != m_floats_right.end(); i++)
				{
					if (fb.pos.left() < i->pos.left())
					{
						m_floats_right.insert(i, std::move(fb));
						inserted = true;
						break;
					}
				}
				if (!inserted)
				{
					m_floats_right.push_back(fb);
				}
			}
			m_cahe_line_right.invalidate();
		}
	}
	else
	{
		box_base::ptr el_parent(m_parent);
		if (el_parent)
		{
			el_parent->place_floated_box(floaded_box, x + m_pos.x, y + m_pos.y);
		}
	}
}

int litehtml::box_model::box_block::render(int x, int y, int max_width, bool second_pass)
{
	int parent_width = max_width;
	document::ptr doc(m_document);

	// calculate outlines
	m_padding.left = m_css_padding.left.calc_percent(parent_width);
	m_padding.right = m_css_padding.right.calc_percent(parent_width);

	m_borders.left = m_css_borders.left.width.calc_percent(parent_width);
	m_borders.right = m_css_borders.right.width.calc_percent(parent_width);

	m_margins.left = m_css_margins.left.calc_percent(parent_width);
	m_margins.right = m_css_margins.right.calc_percent(parent_width);

	m_margins.top = m_css_margins.top.calc_percent(parent_width);
	m_margins.bottom = m_css_margins.bottom.calc_percent(parent_width);

	m_padding.top = m_css_padding.top.calc_percent(parent_width);
	m_padding.bottom = m_css_padding.bottom.calc_percent(parent_width);

	m_pos.move_to(x, y);

	m_pos.x += m_margins.left + m_padding.left + m_borders.left;
	m_pos.y += m_margins.top + m_padding.top + m_borders.top;

	int ret_width = 0;

	def_value<int>	block_width(0);

	style_display display = display_block;
	if (m_element)
	{
		display = m_element->get_display();
	}

	if (display != display_table_cell && !m_css_width.is_predefined())
	{
		int w = calc_width(parent_width);

		if (m_box_sizing == box_sizing_border_box)
		{
			w -= m_padding.width() + m_borders.width();
		}
		ret_width = max_width = block_width = w;
	}
	else
	{
		if (max_width)
		{
			max_width -= m_padding.width() + m_margins.width() + m_borders.width();
		}
	}

	// check for max-width
	if (!m_css_max_width.is_predefined())
	{
		int mw = doc->cvt_units(m_css_max_width, get_font_size(), parent_width);
		if (m_box_sizing == box_sizing_border_box)
		{
			mw -= m_padding.left + m_borders.left + m_padding.right + m_borders.right;
		}
		if (max_width > mw)
		{
			max_width = mw;
		}
	}

	int block_height = 0;

	m_pos.height = 0;

	if (get_predefined_height(block_height))
	{
		m_pos.height = block_height;
	}

	m_floats_left.clear();
	m_floats_right.clear();
	m_cahe_line_left.invalidate();
	m_cahe_line_right.invalidate();

	ret_width = render_children(x, y, max_width, ret_width, second_pass);

	if (block_width.is_default() && get_type() == box_type_element)
	{
		m_pos.width = ret_width;
	}
	else
	{
		m_pos.width = max_width;
	}
	calc_auto_margins(parent_width);

	// add the floats height to the block height
	if (is_flow_root())
	{
		int floats_height = get_floats_height();
		if (floats_height > m_pos.height)
		{
			m_pos.height = floats_height;
		}
	}

	// calculate the final position

	m_pos.move_to(x, y);
	m_pos.x += m_margins.left + m_padding.left + m_borders.left;
	m_pos.y += m_margins.top + m_padding.top + m_borders.top;

	if (get_predefined_height(block_height))
	{
		m_pos.height = block_height;
	}

	int min_height = 0;
	if (!m_css_min_height.is_predefined() && m_css_min_height.units() == css_units_percentage)
	{
		box_base::ptr parent(m_parent);
		if (parent)
		{
			if (parent->get_predefined_height(block_height))
			{
				min_height = m_css_min_height.calc_percent(block_height);
			}
		}
	}
	else
	{
		min_height = (int)m_css_min_height.val();
	}
	if (min_height != 0 && m_box_sizing == box_sizing_border_box)
	{
		min_height -= m_padding.top + m_borders.top + m_padding.bottom + m_borders.bottom;
		if (min_height < 0) min_height = 0;
	}

	/*
	if (m_display == display_list_item)
	{
	const tchar_t* list_image = get_style_property(_t("list-style-image"), true, 0);
	if (list_image)
	{
	tstring url;
	css::parse_css_url(list_image, url);

	size sz;
	const tchar_t* list_image_baseurl = get_style_property(_t("list-style-image-baseurl"), true, 0);
	get_document()->container()->get_image_size(url.c_str(), list_image_baseurl, sz);
	if (min_height < sz.height)
	{
	min_height = sz.height;
	}
	}

	}
	*/

	if (min_height > m_pos.height)
	{
		m_pos.height = min_height;
	}

	int min_width = m_css_min_width.calc_percent(parent_width);

	if (min_width != 0 && m_box_sizing == box_sizing_border_box)
	{
		min_width -= m_padding.left + m_borders.left + m_padding.right + m_borders.right;
		if (min_width < 0) min_width = 0;
	}

	if (min_width != 0)
	{
		if (min_width > m_pos.width)
		{
			m_pos.width = min_width;
		}
		if (min_width > ret_width)
		{
			ret_width = min_width;
		}
	}

	ret_width += m_padding.width() + m_margins.width() + m_borders.width();

	// re-render with new width
	if (ret_width < max_width && !second_pass && have_parent())
	{
		auto display = get_element_display();
		auto position = get_element_position();

		if (display == display_inline_block ||
			m_css_width.is_predefined() &&
			(//m_float != float_none ||
			display == display_table ||
			position == element_position_absolute ||
			position == element_position_fixed
			)
			)
		{
			render(x, y, ret_width, true);
			m_pos.width = ret_width - (m_padding.width() + m_margins.width() + m_borders.width());
		}
	}

	if (is_flow_root() && !second_pass)
	{
		for (const auto& fb : m_floats_left)
		{
			fb.box->apply_relative_shift(fb.box->get_parent()->calc_width(m_pos.width));
			fb.box->update_element();
		}
	}

	update_element();

	return ret_width;
}
