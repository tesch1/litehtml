#pragma once
#include "box_model.h"
#include "box_block.h"

namespace litehtml
{
	namespace box_model
	{
		struct box_line
		{
			typedef std::vector<box_line> vector;

			litehtml::position			pos;
			box_base::vector::iterator	begin;
			box_base::vector::iterator	end;

			box_line() : 
				pos() {}
			box_line(const box_line& val) :
				pos(val.pos),
				begin(val.begin),
				end(val.end) {}
			box_line(box_line&& val) :
				pos(std::move(val.pos)),
				begin(std::move(val.begin)),
				end(std::move(val.end)) {}
		};

		class box_block_with_inlines : public box_block
		{
			box_base::vector	m_element_boxes;
			box_line::vector	m_lines;
		public:
			box_block_with_inlines(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_block(generated_by, doc){}

			virtual void add_box(const box_base::ptr& box) override;

		protected:
			virtual int render_children(int x, int y, int max_width, int ret_width, bool second_pass) override;

			void place_into_lines(const box_base::vector::iterator& box_iter);
		};
	}
}