#include "html.h"
#include "box_model.h"
#include "element.h"
#include "box_block_with_blocks.h"
#include "box_block_with_inlines.h"
#include "box_element.h"
#include "box_inline.h"
#include "document.h"


litehtml::box_model::box_base::~box_base()
{

}

litehtml::box_model::box_base::ptr litehtml::box_model::box_base::create_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc)
{
	std::shared_ptr<box_model::box_base> box;
	if (el->get_float() != float_none)
	{
		box = create_block_box(el, doc);
	}
	else
	{
		switch (el->get_display())
		{
		case display_inline_text:
			box = std::make_shared<box_element>(el, doc);
			break;
		case display_inline:
			box = create_inline_box(el, doc);
			break;
		default:
			box = create_block_box(el, doc);
			break;
		}
	}
	return std::move(box);
}

litehtml::box_model::box_base::ptr litehtml::box_model::box_base::create_block_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc)
{
	box_base::ptr box;
	box_base::vector boxes;
	box_base::ptr child_box;
	box_type boxes_type = box_type_none;
	bool is_blocks_container = false;
	for (const auto& child : el->get_children())
	{
		if (child->is_white_space())
		{
			if (boxes.empty() || boxes.back()->get_type() == box_type_block)
			{
				continue;
			}
		}
		child_box = create_box(child, doc);
		if (child_box)
		{
			if (child_box->get_type() == box_type_block)
			{
				is_blocks_container = true;
			}
			boxes.push_back(std::move(child_box));
		}
	}
	if (is_blocks_container || boxes.empty())
	{
		box = std::make_shared<box_block_with_blocks>(el, doc);
	}
	else
	{
		if (boxes.back()->get_type() == box_type_block)
		{
			box = std::make_shared<box_block_with_blocks>(el, doc);
		}
		else
		{
			box = std::make_shared<box_block_with_inlines>(el, doc);
		}
	}
	for (auto& b : boxes)
	{
		box->add_box(std::move(b));
	}
	return std::move(box);
}

litehtml::box_model::box_base::ptr litehtml::box_model::box_base::create_inline_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc)
{
	box_base::ptr box = std::make_shared<box_inline>(el, doc);
	box_base::ptr child_box;

	for (const auto& child : el->get_children())
	{
		child_box = create_box(child, doc);
		if (child_box)
		{
			box->add_box(std::move(child_box));
		}
	}
	return box;
}

int litehtml::box_model::box_base::render(int x, int y, int max_width, bool second_pass /*= false*/)
{
	return 0;
}

int litehtml::box_model::box_base::calc_width(int defWidth) const
{
	css_length w = get_css_width();
	if (w.is_predefined())
	{
		return defWidth;
	}
	if (w.units() == css_units_percentage)
	{
		if (!have_parent())
		{
			document::ptr doc(m_document);
			if (doc)
			{
				position client_pos;
				doc->container()->get_client_rect(client_pos);
				return w.calc_percent(client_pos.width);
			}
			return defWidth;
		}
		else
		{
			box_base::ptr parent(m_parent);
			if (parent)
			{
				int pw = parent->calc_width(defWidth);
				if (m_element && m_element->is_body())
				{
					pw -= m_margins.width() + m_padding.width() + m_borders.width();
				}
				return w.calc_percent(pw);
			}
			return defWidth;
		}
	}
	document::ptr doc(m_document);
	return 	doc->cvt_units(w, get_font_size());
}

int litehtml::box_model::box_base::get_font_size() const
{
	if (m_element) return m_element->get_font_size();
	box_base::ptr parent(m_parent);
	if (parent)
	{
		return parent->get_font_size();
	}

	document::ptr doc(m_document);
	return 	doc->container()->get_default_font_size();
}

bool litehtml::box_model::box_base::get_predefined_height(int& p_height) const
{
	css_length h = get_css_height();
	if (h.is_predefined())
	{
		p_height = m_pos.height;
		return false;
	}
	if (h.units() == css_units_percentage)
	{
		if (!have_parent())
		{
			document::ptr doc(m_document);

			position client_pos;
			doc->container()->get_client_rect(client_pos);
			p_height = h.calc_percent(client_pos.height);
			return true;
		}
		else
		{
			box_base::ptr parent(m_parent);
			if (parent)
			{
				int ph = 0;
				if (parent->get_predefined_height(ph))
				{
					p_height = h.calc_percent(ph);
					if (m_element && m_element->is_body())
					{
						p_height -= m_margins.height() + m_padding.height() + m_borders.height();
					}
					return true;
				}
				else
				{
					p_height = m_pos.height;
					return false;
				}
			}
			return false;
		}
	}
	document::ptr doc(m_document);
	p_height = doc->cvt_units(h, get_font_size());
	return true;
}

litehtml::css_length litehtml::box_model::box_base::get_css_width() const
{
	return css_length();
}

litehtml::css_length litehtml::box_model::box_base::get_css_height() const
{
	return css_length();
}

litehtml::element_position litehtml::box_model::box_base::get_element_position(css_offsets* offsets) const
{
	if (m_element)
	{
		return m_element->get_element_position(offsets);
	}
	return element_position_static;
}

void litehtml::box_model::box_base::update_element()
{
	if (m_element)
	{
		m_element->set_pos(m_pos);
	}
}

litehtml::style_display litehtml::box_model::box_base::get_element_display() const
{
	if (m_element)
	{
		return m_element->get_display();
	}
	return display_block;
}

void litehtml::box_model::box_base::draw_stacking_context(uint_ptr hdc, int x, int y, const position* clip, bool with_positioned)
{
	//if (!is_visible()) return;

/*
	std::map<int, bool> zindexes;
	if (with_positioned)
	{
		for (elements_vector::iterator i = m_positioned.begin(); i != m_positioned.end(); i++)
		{
			zindexes[(*i)->get_zindex()];
		}

		for (std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
		{
			if (idx->first < 0)
			{
				draw_children(hdc, x, y, clip, draw_positioned, idx->first);
			}
		}
	}
*/
	draw_children(hdc, x, y, clip, draw_block, 0);
//	draw_children(hdc, x, y, clip, draw_floats, 0);
	draw_children(hdc, x, y, clip, draw_inlines, 0);
/*
	if (with_positioned)
	{
		for (std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
		{
			if (idx->first == 0)
			{
				draw_children(hdc, x, y, clip, draw_positioned, idx->first);
			}
		}

		for (std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
		{
			if (idx->first > 0)
			{
				draw_children(hdc, x, y, clip, draw_positioned, idx->first);
			}
		}
	}
*/
}

void litehtml::box_model::box_base::draw_children(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex)
{

}

litehtml::overflow litehtml::box_model::box_base::get_overflow() const
{
	if (m_element)
	{
		return m_element->get_overflow();
	}
	return overflow_visible;
}

void litehtml::box_model::box_base::draw(uint_ptr hdc, int x, int y, const position* clip)
{
	if (m_element)
	{
		m_element->draw(hdc, x, y, clip);
	}
}

bool litehtml::box_model::box_base::is_flow_root() const
{
	return false;
}

void litehtml::box_model::box_base::place_floated_box(const box_base::ptr& box, int x, int y)
{
	box_base::ptr parent(m_parent);
	if (parent)
	{
		parent->place_floated_box(box, x, y);
	}
}

litehtml::element_float litehtml::box_model::box_base::get_float() const
{
	return float_none;
}

litehtml::element_clear litehtml::box_model::box_base::get_clear() const
{
	return clear_none;
}

void litehtml::box_model::box_base::apply_relative_shift(int parent_width)
{
	css_offsets offsets;
	if (get_element_position(&offsets) == element_position_relative)
	{
		box_base::ptr parent_ptr(m_parent);
		if (!offsets.left.is_predefined())
		{
			m_pos.x += offsets.left.calc_percent(parent_width);
		}
		else if (!offsets.right.is_predefined())
		{
			m_pos.x -= offsets.right.calc_percent(parent_width);
		}
		if (!offsets.top.is_predefined())
		{
			int h = 0;

			if (offsets.top.units() == css_units_percentage)
			{
				box_base::ptr el_parent(m_parent);
				if (el_parent)
				{
					el_parent->get_predefined_height(h);
				}
			}

			m_pos.y += offsets.top.calc_percent(h);
		}
		else if (!offsets.bottom.is_predefined())
		{
			int h = 0;

			if (offsets.top.units() == css_units_percentage)
			{
				box_base::ptr el_parent(m_parent);
				if (el_parent)
				{
					el_parent->get_predefined_height(h);
				}
			}

			m_pos.y -= offsets.bottom.calc_percent(h);
		}
	}
}

int litehtml::box_model::box_base::get_floats_height(element_float el_float /*= float_none*/) const
{
	box_base::ptr el_parent = get_parent();
	if (el_parent)
	{
		int h = el_parent->get_floats_height(el_float);
		return h - m_pos.y;
	}
	return 0;
}

void litehtml::box_model::box_base::fetch_by_type(box_type bt, box_base::vector& items)
{
	for (auto& box : m_children)
	{
		if (box->get_type() == bt)
		{
			items.push_back(box);
		}
		else
		{
			box->fetch_by_type(bt, items);
		}
	}
}

void litehtml::box_model::box_base::get_element_content_size(litehtml::size& sz, int max_width) const
{
	if (m_element)
	{
		m_element->get_content_size(sz, max_width);
	}
	else
	{
		sz.width = sz.height = 0;
	}
}

bool litehtml::box_model::box_base::is_white_space() const
{
	if (m_element)
	{
		return m_element->is_white_space();
	}
	return false;
}

bool litehtml::box_model::box_base::is_break() const
{
	if (m_element)
	{
		return m_element->is_break();
	}
	return false;
}

int litehtml::box_model::box_base::line_height() const
{
	if (m_element)
	{
		return m_element->line_height();
	}
	box_base::ptr el_parent = get_parent();
	if (el_parent)
	{
		return el_parent->line_height();
	}
	return 0;
}

litehtml::uint_ptr litehtml::box_model::box_base::get_font(font_metrics* fm /*= 0*/) const
{
	if (m_element)
	{
		return m_element->get_font(fm);
	}
	box_base::ptr el_parent = get_parent();
	if (el_parent)
	{
		return el_parent->get_font(fm);
	}
	return 0;
}

const litehtml::tchar_t* litehtml::box_model::box_base::get_style_property(const tchar_t* name, bool inherited, const tchar_t* def /*= 0*/) const
{
	if (m_element)
	{
		return m_element->get_style_property(name, inherited, def);
	}
	if (inherited)
	{
		box_base::ptr el_parent = get_parent();
		if (el_parent)
		{
			return el_parent->get_style_property(name, inherited, def);
		}
	}
	return def;
}

litehtml::vertical_align litehtml::box_model::box_base::get_vertical_align() const
{
	if (m_element)
	{
		return m_element->get_vertical_align();
	}
	box_base::ptr el_parent = get_parent();
	if (el_parent)
	{
		return el_parent->get_vertical_align();
	}
	return va_baseline;
}

int litehtml::box_model::box_base::get_base_line() const
{
	return 0;
}
