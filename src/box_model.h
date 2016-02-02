#pragma once

namespace litehtml
{
	class element;

	namespace box_model
	{
		enum box_type
		{
			box_type_none,
			box_type_block,
			box_type_inline,
			box_type_element,
		};

		class box_base : public std::enable_shared_from_this<box_base>
		{
		public:
			typedef std::shared_ptr<box_base>	ptr;
			typedef std::weak_ptr<box_base>		weak_ptr;
			typedef std::vector<box_base::ptr>	vector;
		protected:
			box_base::weak_ptr			m_parent;	// containing box
			std::shared_ptr<element>	m_element;	// element who generated this box
			box_base::vector			m_children;	// children boxes
			std::weak_ptr<document>		m_document;

			position	m_pos;
			margins		m_margins;
			margins		m_padding;
			margins		m_borders;
		public:
			box_base(const std::shared_ptr<element>& generated_by, const std::shared_ptr<document>& doc) :
				m_element(generated_by), m_document(doc) {}
			virtual ~box_base();

			static std::shared_ptr<box_base> create_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc);
			void set_parent(const box_base::ptr& parent);
			bool is_anonymous() const { return (m_element == nullptr); }
			int get_font_size() const;
			int calc_width(int defWidth) const;
			bool get_predefined_height(int& p_height) const;
			element_position get_element_position(css_offsets* offsets = 0) const;
			style_display get_element_display() const;
			bool have_parent() const;
			void update_element();
			void draw_stacking_context(uint_ptr hdc, int x, int y, const position* clip, bool with_positioned);
			void draw(uint_ptr hdc, int x, int y, const position* clip);
			box_base::ptr get_parent() const;
			void get_element_content_size(litehtml::size& sz, int max_width) const;

			int top() const;
			int left() const;
			int bottom() const;
			int right() const;
			int width() const;
			int height() const;
			int content_top() const;
			int content_left() const;
			int content_bottom() const;
			int content_right() const;
			int content_width() const;
			int content_height() const;

			int content_margins_top() const;
			int content_margins_left() const;
			int content_margins_bottom() const;
			int content_margins_right() const;

			const margins& get_margins() const;
			const margins& get_paddings() const;
			const margins& get_borders() const;

			void move(int dx, int dy);
			void move_to(int x, int y);
			void apply_relative_shift(int parent_width);
			void fetch_by_type(box_type bt, box_base::vector& items);

			bool is_white_space() const;
			bool is_break() const;
			int line_height() const;
			uint_ptr get_font(font_metrics* fm = 0) const;
			vertical_align get_vertical_align() const;

			const tchar_t* get_style_property(const tchar_t* name, bool inherited, const tchar_t* def = 0) const;

			virtual box_type get_type() const = 0;
			virtual void add_box(const std::shared_ptr<box_base>& box) = 0;
			virtual void init_from_element() = 0;
			virtual int render(int x, int y, int max_width, bool second_pass = false);
			virtual css_length get_css_width() const;
			virtual css_length get_css_height() const;
			virtual element_float get_float() const;
			virtual element_clear get_clear() const;
			virtual void draw_children(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex);
			virtual bool is_flow_root() const;
			virtual overflow get_overflow() const;
			virtual void place_floated_box(const box_base::ptr& box, int x, int y);
			virtual int get_floats_height(element_float el_float = float_none) const;
			virtual int get_base_line() const;

		protected:
			static box_base::ptr create_block_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc);
			static box_base::ptr create_inline_box(const std::shared_ptr<element>& el, const std::shared_ptr<document>& doc);
		};

		inline box_base::ptr box_base::get_parent() const
		{
			return box_base::ptr(m_parent);
		}

		inline void box_base::set_parent(const box_base::ptr& parent)
		{
			m_parent = parent;
		}

		inline int box_base::top() const
		{
			return m_pos.top() - m_margins.top - m_padding.top - m_borders.top;
		}

		inline int box_base::left() const
		{
			return m_pos.left() - m_margins.left - m_padding.left - m_borders.left;
		}

		inline int box_base::bottom() const
		{
			return m_pos.bottom() + m_margins.bottom + m_padding.bottom + m_borders.bottom;
		}

		inline int box_base::right() const
		{
			return m_pos.right() + m_margins.right + m_padding.right + m_borders.right;
		}

		inline int box_base::width() const
		{
			return m_pos.width + m_margins.width() + m_padding.width() + m_borders.width();
		}

		inline int box_base::height() const
		{
			return m_pos.height + m_margins.height() + m_padding.height() + m_borders.height();
		}


		inline int box_base::content_top() const
		{
			return m_pos.top();
		}

		inline int box_base::content_left() const
		{
			return m_pos.left();
		}

		inline int box_base::content_bottom() const
		{
			return m_pos.bottom();
		}

		inline int box_base::content_right() const
		{
			return m_pos.right();
		}

		inline int box_base::content_width() const
		{
			return m_pos.width;
		}

		inline int box_base::content_height() const
		{
			return m_pos.height;
		}

		inline int box_base::content_margins_top() const
		{
			return m_margins.top + m_padding.top + m_borders.top;
		}
		inline int box_base::content_margins_left() const
		{
			return m_margins.left + m_padding.left + m_borders.left;
		}
		inline int box_base::content_margins_bottom() const
		{
			return m_margins.bottom + m_padding.bottom + m_borders.bottom;
		}
		inline int box_base::content_margins_right() const
		{
			return m_margins.right + m_padding.right + m_borders.right;
		}

		inline const margins& box_base::get_margins() const
		{
			return m_margins;
		}

		inline const margins& box_base::get_paddings() const
		{
			return m_padding;
		}

		inline const margins& box_base::get_borders() const
		{
			return m_borders;
		}

		inline bool box_base::have_parent() const
		{
			return !m_parent.expired();
		}

		inline void box_base::move(int dx, int dy)
		{
			m_pos.x += dx;
			m_pos.y += dy;
		}

		inline void box_base::move_to(int x, int y)
		{
			m_pos.x = x;
			m_pos.y = y;
		}

	}
}