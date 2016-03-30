#include "html.h"
#include "box_block_with_blocks.h"
#include "box_block_with_inlines.h"
#include "document.h"

void litehtml::box_model::box_block_with_blocks::add_box(const box_base::ptr& box)
{
	if (box->get_type() == box_type_block)
	{
		push_child_box(box);
	}
	else
	{
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
			box_base::ptr anon_box = std::make_shared<box_block_with_inlines>(nullptr, std::shared_ptr<document>(m_document));
			push_child_box(anon_box);
			anon_box->add_box(box);
		}
	}
}

int litehtml::box_model::box_block_with_blocks::render_children(int x, int y, int max_width, int ret_width, bool second_pass)
{
	int top = 0;

	box_base::ptr last_box;

	for (auto& box : m_children)
	{
		int rw = box->render(x, top, max_width);
		if (rw > ret_width)
		{
			ret_width = rw;
		}
		if (!last_box)
		{
			// collapse top margin
			if (m_padding.top == 0 && m_borders.top == 0)
			{
				if (m_margins.top >= 0 && box->get_margins().top >= 0)
				{
					if (m_margins.top < box->get_margins().top)
					{
						m_margins.top = box->get_margins().top;
					}
					if (box->get_margins().top > 0)
					{
						box->move(0, -box->get_margins().top);
						box->update_element();
					}
				}
			}
		}
		else if (last_box->get_margins().bottom > 0 && box->get_margins().top > 0)
		{
			// collapse margins between blocks
			int shift = 0;
			if (last_box->get_margins().bottom < box->get_margins().top)
			{
				shift = last_box->get_margins().bottom - box->get_margins().top;
			}
			else
			{
				shift = -box->get_margins().top;
			}
			if (shift != 0)
			{
				box->move(0, shift);
				box->update_element();
			}
		}

		last_box = box;
		top = box->bottom();
		if (m_pos.height < top)
		{
			m_pos.height = top;
		}
	}

	// collapse bottom margin
	if (last_box && m_padding.bottom == 0 && m_borders.bottom == 0)
	{
		if (m_margins.bottom >= 0 && last_box->get_margins().bottom >= 0)
		{
			if (m_margins.bottom < last_box->get_margins().bottom)
			{
				m_margins.bottom = last_box->get_margins().bottom;
			}
			m_pos.height -= last_box->get_margins().bottom;
		}
	}

	return ret_width;
}

void litehtml::box_model::box_block_with_blocks::draw_children(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex)
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

	position browser_wnd;
	doc->container()->get_client_rect(browser_wnd);

	box_base::ptr el;
	for (auto& item : m_children)
	{
		el = item;
		if (true /*el->is_visible()*/)
		{
			switch (flag)
			{
/*
			case draw_positioned:
				if (el->is_positioned() && el->get_zindex() == zindex)
				{
					if (el->get_element_position() == element_position_fixed)
					{
						el->draw(hdc, browser_wnd.x, browser_wnd.y, clip);
						el->draw_stacking_context(hdc, browser_wnd.x, browser_wnd.y, clip, true);
					}
					else
					{
						el->draw(hdc, pos.x, pos.y, clip);
						el->draw_stacking_context(hdc, pos.x, pos.y, clip, true);
					}
					el = 0;
				}
				break;
*/
			case draw_block:
				el->draw(hdc, pos.x, pos.y, clip);
				break;
/*
			case draw_floats:
				if (el->get_float() != float_none && !el->is_positioned())
				{
					el->draw(hdc, pos.x, pos.y, clip);
					el->draw_stacking_context(hdc, pos.x, pos.y, clip, false);
					el = 0;
				}
				break;
			case draw_inlines:
				if (el->is_inline_box() && el->get_float() == float_none && !el->is_positioned())
				{
					el->draw(hdc, pos.x, pos.y, clip);
					if (el->get_display() == display_inline_block)
					{
						el->draw_stacking_context(hdc, pos.x, pos.y, clip, false);
						el = 0;
					}
				}
				break;
*/
			default:
				break;
			}

			if (el)
			{
/*
				if (flag == draw_positioned)
				{
					if (!el->is_positioned())
					{
						el->draw_children(hdc, pos.x, pos.y, clip, flag, zindex);
					}
				}
				else
*/
				{
/*
					if (el->get_float() == float_none &&
						el->get_display() != display_inline_block &&
						!el->is_positioned())
*/
					{
						el->draw_children(hdc, pos.x, pos.y, clip, flag, zindex);
					}
				}
			}
		}
	}

	if (get_overflow() > overflow_visible)
	{
		doc->container()->del_clip();
	}
}

int litehtml::box_model::box_block_with_blocks::calculate_base_line() const
{
	if (!m_children.empty())
	{
		return (m_pos.height + content_margins_bottom()) - (m_children.back()->bottom() - m_children.back()->calculate_base_line());
	}
	return content_margins_bottom();
}
