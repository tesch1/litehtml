#pragma once
#include "box_model.h"

namespace litehtml
{
	namespace box_model
	{
		class box_inline : public box_base
		{
		public:
			box_inline(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_base(generated_by, doc) {}
			virtual ~box_inline();

			virtual box_type get_type() const override;
			virtual void add_box(const box_base::ptr& box) override;
			virtual void init_from_element() override;
		};
	}
}
