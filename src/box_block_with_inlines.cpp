#include "html.h"
#include "box_block_with_inlines.h"
#include "box_inline.h"
#include "document.h"

void litehtml::box_model::box_block_with_inlines::add_box(const box_base::ptr& box)
{
	if (box->get_type() == box_type_inline)
	{
		box->fetch_by_type(box_type_element, m_element_boxes);
		m_children.push_back(box);
		box->set_parent(shared_from_this());
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
			anon_box->add_box(box);
			m_children.push_back(anon_box);
			anon_box->set_parent(shared_from_this());
		}
	}
}

int litehtml::box_model::box_block_with_inlines::render_children(int x, int y, int max_width, int ret_width, bool second_pass)
{
	m_lines.clear();

	for (auto el_box = m_element_boxes.begin(); el_box != m_element_boxes.end(); el_box++)
	{
		if (!second_pass)
		{
			(*el_box)->render(0, 0, max_width, second_pass);
		}
		place_into_lines(el_box);
		if (m_lines.empty())
		{
			m_lines.emplace_back();
			m_lines.back().begin = el_box;
			m_lines.back().end = el_box;
		}
		if ((*el_box)->width() + m_lines.back().pos.width > max_width)
		{
			int bottom = m_lines.back().pos.bottom();
			m_lines.emplace_back();
			m_lines.back().end = el_box;
		}
	}
}

void litehtml::box_model::box_block_with_inlines::place_into_lines(const box_base::vector::iterator& box_iter)
{

}

