#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include <tuple>

#include "Common.h"

template<int TId, typename TUserState>
struct RectangleState
{
	STATE_PROPERTY(SDL_Point, size);
	STATE_PROPERTY(SDL_Point, position);
	STATE_PROPERTY(SDL_Color, color);
};

template<Operation TOperation, typename TContext, typename ...TProperties>
struct RectangleLogic
{
};

template<typename TContext, typename ...TProperties>
struct RectangleLogic<Operation::Initialize, TContext, TProperties...>
{
	static auto invoke(const TContext &context, TProperties &...)
	{
		return context_prepend(RectangleState<Level<TContext>, UserState<TContext>>(), context);
	}
};

template<typename TContext, typename ...TProperties>
struct RectangleLogic<Operation::Update, TContext, TProperties...>
{
	static auto invoke(const TContext &context, const TProperties &...properties)
	{
		const auto &rectangle = std::get<RectangleState<Level<TContext>, UserState<TContext>>>(context.state);

		return repack(context,
			apply_properties(rectangle, properties...)
		);
	}
};

template<typename TContext, typename ...TProperties>
struct RectangleLogic<Operation::Draw, TContext, TProperties...>
{
	static auto invoke(const TContext &context, TProperties &...)
	{
		const auto &rectangle = std::get<RectangleState<Level<TContext>, UserState<TContext>>>(context.state);
		const auto &root = std::get<RootState>(context.state);

		const SDL_Rect bounds = { rectangle.position.x, rectangle.position.y, rectangle.size.x, rectangle.size.y };

		SDL_FillRect(root.surface, &bounds, *(Uint32 *)&rectangle.color);

		return context;
	}
};

template<Operation TOperation, typename TContext, typename ...TProperties>
auto Rectangle(const TContext &context, TProperties ...properties)
{
	return RectangleLogic<TOperation, decltype(level_up(context)), TProperties...>::invoke(level_up(context), properties...);
}

#endif // RECTANGLE_H
