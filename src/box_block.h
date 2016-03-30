#pragma once
#include "box_model.h"

namespace litehtml
{
	namespace box_model
	{
		struct floated_box;

		class box_block : public box_base
		{
		protected:
			css_margins		m_css_margins;
			css_margins		m_css_padding;
			css_borders		m_css_borders;
			css_length		m_css_width;
			css_length		m_css_height;
			css_length		m_css_min_width;
			css_length		m_css_min_height;
			css_length		m_css_max_width;
			css_length		m_css_max_height;
			css_offsets		m_css_offsets;
			box_sizing		m_box_sizing;
			element_float	m_float;
			element_clear	m_clear;
			overflow		m_overflow;

			std::vector<floated_box>	m_floats_left;
			std::vector<floated_box>	m_floats_right;
			int_int_cache				m_cache_line_left;
			int_int_cache				m_cache_line_right;
		public:
			box_block(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_base(generated_by, doc),
				m_box_sizing(box_sizing_content_box),
				m_float(float_none),
				m_clear(clear_none),
				m_overflow(overflow_visible){}

			virtual box_type get_type() const override;
			virtual void init_from_element() override;
			virtual css_length get_css_width() const override;
			virtual css_length get_css_height() const override;
			virtual bool is_flow_root() const override;
			virtual overflow get_overflow() const override;
			virtual element_float get_float() const;
			virtual element_clear get_clear() const;
			virtual void place_floated_box(const box_base::ptr& box, int x, int y) override;
			virtual int render(int x, int y, int max_width, bool second_pass) override;
			virtual int get_floats_height(element_float el_float = float_none) const override;

		protected:
			void calc_auto_margins(int parent_width);
			virtual int render_children(int x, int y, int max_width, int ret_width, bool second_pass) = 0;
		};

		struct floated_box
		{
			typedef std::vector<floated_box>	vector;

			position		pos;
			element_float	float_side;
			element_clear	clear_floats;
			std::shared_ptr<box_base>	box;

			floated_box() = default;
			floated_box(const floated_box& val)
			{
				pos = val.pos;
				float_side = val.float_side;
				clear_floats = val.clear_floats;
				box = val.box;
			}
			floated_box& operator=(const floated_box& val)
			{
				pos = val.pos;
				float_side = val.float_side;
				clear_floats = val.clear_floats;
				box = val.box;
				return *this;
			}
			floated_box(floated_box&& val)
			{
				pos = val.pos;
				float_side = val.float_side;
				clear_floats = val.clear_floats;
				box = std::move(val.box);
			}
			void operator=(floated_box&& val)
			{
				pos = val.pos;
				float_side = val.float_side;
				clear_floats = val.clear_floats;
				box = std::move(val.box);
			}
		};

	}
}
