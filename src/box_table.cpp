#include "html.h"
#include "box_table.h"

litehtml::box_model::box_type litehtml::box_model::box_table::get_type() const
{
	if (m_element->get_display() == display_inline_table)
	{
		return box_type_element;
	}
	return box_type_block;
}

void litehtml::box_model::box_table::add_box(const box_base::ptr& box)
{
	if (box->get_type() == box_type_block)
	{
		push_child_box(box);
	}
}
