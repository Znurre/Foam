#ifndef MOUSEAREA_H
#define MOUSEAREA_H

#include "Common.h"
#include "Item.h"

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

template<typename ...TParameters>
struct MouseArea : public Item<MouseAreaLogic, TParameters...>
{
	MouseArea(const TParameters &...parameters)
		: Item<MouseAreaLogic, TParameters...>(parameters...)
	{
	}
};

#endif // MOUSEAREA_H
