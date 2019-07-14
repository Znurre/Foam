#ifndef TEXT_H
#define TEXT_H

#include <iostream>

#include <glm/gtx/matrix_transform_2d.hpp>

#include "Common.h"
#include "Item.h"
#include "Algorithms.h"

enum Alignment
{
	// Horizontal
	AlignLeft = 0x0001,
	AlignRight = 0x0002,
	AlignHCenter = 0x0004,
	AlignJustify = 0x0008,

	// Vertical
	AlignTop = 0x0020,
	AlignBottom = 0x0040,
	AlignVCenter =0x0080,
	AlignBaseline = 0x0100,
};

template<int TId, typename TUserState>
struct TextState : public DrawableControl
{
	STATE_PROPERTY(glm::vec2, size)
	STATE_PROPERTY(glm::vec2, position)
	STATE_PROPERTY(uint, color)
	STATE_PROPERTY(std::string, text)
	STATE_PROPERTY(std::string, previous_text)
	STATE_PROPERTY(std::vector<DrawCommand>, draw_commands)
	STATE_PROPERTY(int, alignment)
};

template<Operation TOperation>
struct TextLogic
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &text = read_control_state<TextState>(context);

		return repack(context, text
			.with_draw_commands({})
			.with_previous_text(std::string())
			);
	}
};

template<>
struct TextLogic<Operation::Initialize>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context_prepend(TextState<get_level_v<TContext>, get_user_state_t<TContext>>(), context);
	}
};

template<>
struct TextLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &text = read_control_state<TextState>(context);

		return repack(context,
			apply_properties(properties, text)
		);
	}
};

template<>
struct TextLogic<Operation::Draw>
{
	struct GlyphTransformer
	{
		GlyphTransformer(const RootState &root)
			: root(root)
		{
		}

		Glyph operator ()(const char &character)
		{
			return root.glyphs[character - 32u];
		}

		const RootState &root;
	};

	template<typename TState>
	struct TextTransformer
	{
		TextTransformer(const TState &text)
			: text(text)
		{
		}

		std::tuple<int, DrawCommand> operator ()(const Glyph &glyph, const std::tuple<int, DrawCommand> &previous) const
		{
			const auto &size = glm::vec2(glyph.bounds.z, glyph.bounds.w);
			const auto &position = glm::vec2(text.position.x + std::get<int>(previous), text.position.y + glyph.offset);
			const auto &uv = glyph.bounds / float(TEXTURE_SIZE);

			const auto &command = DrawCommand()
				.with_uv(uv)
				.with_matrix(glm::scale(glm::translate(glm::mat3(1.0f), position), size))
				.with_color(text.color);

			return { std::get<int>(previous) + glyph.ax, command };
		}

		const TState &text;
	};

	static DrawCommand transform(const std::tuple<int, DrawCommand> &tuple)
	{
		return std::get<DrawCommand>(tuple);
	}

	static long accumulate_width(long width, const Glyph &glyph)
	{
		return width + glyph.ax;
	}

	static float accumulate_height(float height, const Glyph &glyph)
	{
		return std::max(height, glyph.bounds.w);
	}

	template<typename TState>
	static glm::ivec2 get_vertical_position(const TState &text, uint font_height, const glm::ivec2 &position)
	{
		if (text.alignment & AlignTop)
		{
			return position;
		}

		if (text.alignment & AlignVCenter)
		{
			return glm::ivec2(position.x, position.y + (text.size.y - font_height) / 2);
		}

		return glm::ivec2(position.x, position.y + (text.size.y - font_height));
	}

	template<typename TState>
	static glm::ivec2 get_horizontal_position(const TState &text, const std::vector<Glyph> &glyphs, const glm::ivec2 &position)
	{
		if (text.alignment & AlignLeft)
		{
			return position;
		}

		const long width = std::accumulate(std::begin(glyphs), std::end(glyphs), 0, &accumulate_width);

		if (text.alignment & AlignHCenter)
		{
			return glm::ivec2(position.x + (text.size.x - width) / 2, position.y);
		}

		return glm::ivec2(position.x + (text.size.x - width), position.y);
	}

	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &control = read_control_state<TextState>(context);
		const auto &root = read_root_state(context);

		if (control.text.empty())
		{
			return context;
		}

		if (control.text == control.previous_text)
		{
			return context;
		}

		const GlyphTransformer glyphTransformer(root);

		std::vector<std::tuple<int, DrawCommand>> blueprints;

		std::vector<Glyph> glyphs;
		std::vector<DrawCommand> commands;

		std::transform(std::begin(control.text)
			, std::end(control.text)
			, std::back_inserter(glyphs)
			, glyphTransformer
			);

		const glm::ivec2 position = get_vertical_position(control, root.font_height
			, get_horizontal_position(control, glyphs, control.position)
			);

		const TextTransformer<decltype(control)> textTransformer(control.with_position(position));

		fold_transform(std::begin(glyphs)
			, std::end(glyphs)
			, std::back_inserter(blueprints)
			, textTransformer
			);

		std::transform(std::begin(blueprints)
			, std::end(blueprints)
			, std::back_inserter(commands)
			, transform
			);

		return repack(context, control
			.with_draw_commands(commands)
			.with_previous_text(control.text)
			);
	}
};

template<typename ...TParameters>
struct Text : public Item<TextLogic, TParameters...>
{
	Text(const TParameters &...parameters)
		: Item<TextLogic, TParameters...>(parameters...)
	{
	}
};


#endif // TEXT_H
