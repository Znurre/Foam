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

template<Operation TOperation>
struct MouseAreaLogic
{
};

template<>
struct MouseAreaLogic<Operation::Initialize>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context_prepend(MouseAreaState<Level<TContext>, UserState<TContext>>(), context);
	}
};

template<>
struct MouseAreaLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &rectangle = std::get<MouseAreaState<Level<TContext>, UserState<TContext>>>(context.state);

		return repack(context,
			apply_properties(rectangle, properties)
		);
	}
};

template<>
struct MouseAreaLogic<Operation::Draw>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context;
	}
};

template<typename TContext, typename ...TProperties>
auto MouseArea(const TContext &context, TProperties ...properties)
{
	return MouseAreaLogic<Op<TContext>>::invoke(level_up(context), std::make_tuple(properties...));
}

#endif // MOUSEAREA_H
