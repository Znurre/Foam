#ifndef TEXT_H
#define TEXT_H

#include <vector>

#include <SDL.h>

#include <glm/gtx/matrix_transform_2d.hpp>

#include "Common.h"
#include "Item.h"
#include "Algorithms.h"

template<int TId, typename TUserState>
struct TextState : public DrawableControl
{
	STATE_PROPERTY(glm::vec2, size)
	STATE_PROPERTY(glm::vec2, position)
	STATE_PROPERTY(uint, color)
	STATE_PROPERTY(std::string, text)
	STATE_PROPERTY(std::vector<DrawCommand>, draw_commands)
};

template<Operation TOperation>
struct TextLogic
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &text = read_control_state<TextState>(context);

		return repack(context,
			text.with_draw_commands({})
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
	template<typename TContext>
	struct TextTransformer
	{
		TextTransformer(const TContext &context)
			: context(context)
		{
		}

		std::tuple<int, DrawCommand> operator ()(const char &character, const std::tuple<int, DrawCommand> &previous) const
		{
			const auto &text = read_control_state<TextState>(context);
			const auto &root = read_root_state(context);

			const auto &glyph = root.glyphs[character - 32];

			const auto &size = glm::vec2(glyph.bounds.z, glyph.bounds.w);
			const auto &position = glm::vec2(text.position.x + std::get<int>(previous), text.position.y + glyph.offset);
			const auto &uv = glyph.bounds / float(TEXTURE_SIZE);

			const auto &command = DrawCommand()
				.with_uv(uv)
				.with_matrix(glm::scale(glm::translate(glm::mat3(1.0f), position), size))
				.with_color(text.color);

			return { std::get<int>(previous) + glyph.ax, command };
		}

		const TContext &context;
	};

	static DrawCommand transform(const std::tuple<int, DrawCommand> &tuple)
	{
		return std::get<DrawCommand>(tuple);
	}

	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &text = read_control_state<TextState>(context);

		const TextTransformer<TContext> transformer(context);

		std::vector<std::tuple<int, DrawCommand>> blueprints;
		std::vector<DrawCommand> commands;

		fold_transform(std::begin(text.text)
			, std::end(text.text)
			, std::back_inserter(blueprints)
			, transformer
			);

		std::transform(std::begin(blueprints)
			, std::end(blueprints)
			, std::back_inserter(commands)
			, transform
			);

		return repack(context
			, text.with_draw_commands(commands)
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
