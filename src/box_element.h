#pragma once
#include "box_model.h"

namespace litehtml
{
	namespace box_model
	{
		class box_element : public box_base
		{
		public:
			box_element(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_base(generated_by, doc) {}
			virtual ~box_element();

			virtual box_type get_type() const override;
			virtual void add_box(const box_base::ptr& box) override;
			virtual void init_from_element() override;
			virtual int render(int x, int y, int max_width, bool second_pass) override;
			virtual int calculate_base_line() const override;
		};
	}
}
