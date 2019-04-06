#ifndef MOUSEAREA_H
#define MOUSEAREA_H

#include "Common.h"

template<int TId, typename TUserState>
struct MouseAreaState
{
	STATE_PROPERTY(SDL_Point, size);
	STATE_PROPERTY(SDL_Point, position);
	STATE_PROPERTY(Callback<TUserState>, on_mouse_over);
	STATE_PROPERTY(Callback<TUserState>, on_click);
};

template<Operation TOperation, typename TContext, typename ...TProperties>
struct MouseAreaLogic
{
};

template<typename TContext, typename ...TProperties>
struct MouseAreaLogic<Operation::Initialize, TContext, TProperties...>
{
	static auto invoke(const TContext &context, TProperties &...)
	{
		return context_prepend(MouseAreaState<Level<TContext>, UserState<TContext>>(), context);
	}
};

template<typename TContext, typename ...TProperties>
struct MouseAreaLogic<Operation::Update, TContext, TProperties...>
{
	static auto invoke(const TContext &context, const TProperties &...properties)
	{
		const auto &rectangle = std::get<MouseAreaState<Level<TContext>, UserState<TContext>>>(context.state);

		return repack(context,
			apply_properties(rectangle, properties...)
		);
	}
};

template<typename TState, typename ...TProperties>
struct MouseAreaLogic<Operation::Draw, TState, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		return state;
	}
};

template<typename TContext, typename ...TProperties>
auto MouseArea(const TContext &context, TProperties ...properties)
{
	return MouseAreaLogic<Op<TContext>, decltype(level_up(context)), TProperties...>::invoke(level_up(context), properties...);
}

#endif // MOUSEAREA_H
