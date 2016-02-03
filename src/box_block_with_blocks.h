#pragma once
#include "box_model.h"
#include "box_block.h"

namespace litehtml
{
	namespace box_model
	{
		class box_block_with_blocks : public box_block
		{
		public:
			box_block_with_blocks(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_block(generated_by, doc) {}

			virtual void add_box(const box_base::ptr& box) override;
			virtual int render_children(int x, int y, int max_width, int ret_width, bool second_pass) override;
			virtual void draw_children(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex) override;
			virtual int calculate_base_line() const override;
		};
	}
}