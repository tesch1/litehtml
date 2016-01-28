#pragma once
#include "box_model.h"

namespace litehtml
{
	namespace box_model
	{
		class box_table : public box_base
		{
		public:
			box_table(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				box_base(generated_by, doc) {}
			virtual ~box_table();

			virtual box_type get_type() const override;
			virtual void add_box(const box_base::ptr& box) override;
		};
	}
}
