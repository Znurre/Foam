#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <SDL.h>

#include <tuple>

#include "Common.h"
#include "Item.h"

template<int TId, typename TUserState>
struct RectangleState
{
	STATE_PROPERTY(SDL_Point, size)
	STATE_PROPERTY(SDL_Point, position)
	STATE_PROPERTY(SDL_Color, color)
};

template<Operation TOperation>
struct RectangleLogic
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context;
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
		const auto &rectangle = std::get<RectangleState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);

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
		const auto &rectangle = std::get<RectangleState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);
		const auto &root = std::get<RootState>(context.state);

		const SDL_Rect bounds = { rectangle.position.x, rectangle.position.y, rectangle.size.x, rectangle.size.y };

		SDL_FillRect(root.surface, &bounds, *(Uint32 *)&rectangle.color);

		return context;
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
