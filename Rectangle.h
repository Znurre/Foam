#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <SDL.h>

#include <glm/gtx/matrix_transform_2d.hpp>

#include <tuple>
#include <vector>

#include "Common.h"
#include "Item.h"

template<int TId, typename TUserState>
struct RectangleState : public DrawableControl
{
	STATE_PROPERTY(glm::vec2, size)
	STATE_PROPERTY(glm::vec2, position)
	STATE_PROPERTY(uint, color)
	STATE_PROPERTY(immutable_vector<DrawCommand>, draw_commands)
};

template<Operation TOperation>
struct RectangleLogic
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &rectangle = read_control_state<RectangleState>(context);

		return repack(context,
			rectangle.with_draw_commands({})
		);
	}
};

template<>
struct RectangleLogic<Operation::Initialize>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context_prepend(RectangleState<get_level_v<TContext>, get_user_state_t<TContext>>(), context);
	}
};

template<>
struct RectangleLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &rectangle = read_control_state<RectangleState>(context);

		return repack(context,
			apply_properties(properties, rectangle)
		);
	}
};

template<>
struct RectangleLogic<Operation::Draw>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &rectangle = read_control_state<RectangleState>(context);

		const auto &draw_command = DrawCommand()
			.with_matrix(glm::scale(glm::translate(glm::mat3(1.0f), rectangle.position), rectangle.size))
			.with_color(rectangle.color);

		return repack(context
			, rectangle.with_draw_commands({ draw_command })
		);
	}
};

template<typename ...TParameters>
struct Rectangle : public Item<RectangleLogic, TParameters...>
{
	Rectangle(const TParameters &...parameters)
		: Item<RectangleLogic, TParameters...>(parameters...)
	{
	}
};

#endif // RECTANGLE_H
