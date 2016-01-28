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
