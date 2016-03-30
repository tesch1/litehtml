#include "html.h"
#include "box_inline.h"

litehtml::box_model::box_inline::~box_inline()
{

}

litehtml::box_model::box_type litehtml::box_model::box_inline::get_type() const
{
	return box_type_inline;
}

void litehtml::box_model::box_inline::add_box(const box_base::ptr& box)
{
	push_child_box(box);
}

void litehtml::box_model::box_inline::init_from_element()
{

}
