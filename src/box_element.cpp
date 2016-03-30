#include "html.h"
#include "box_element.h"

litehtml::box_model::box_element::~box_element()
{

}

litehtml::box_model::box_type litehtml::box_model::box_element::get_type() const
{
	return box_type_element;
}

void litehtml::box_model::box_element::add_box(const box_base::ptr& box)
{
	// this box does not have children
}

void litehtml::box_model::box_element::init_from_element()
{

}

int litehtml::box_model::box_element::render(int x, int y, int max_width, bool second_pass)
{
	int ret = 0;
	if (m_element && !second_pass)
	{
		litehtml::size sz;
		m_element->get_content_size(sz, max_width);
		ret = sz.width;
		m_pos = sz;
	}
	m_pos.x = x;
	m_pos.y = y;
	update_element();
	return ret;
}

int litehtml::box_model::box_element::calculate_base_line() const
{
	if (get_element_display() == display_inline_text)
	{
		font_metrics fm;
		get_font(&fm);
		return fm.base_line();
	}
	return 0;
}
