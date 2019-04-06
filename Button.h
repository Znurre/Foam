#ifndef BUTTON_H
#define BUTTON_H

#include <QApplication>
#include <SDL.h>
#include <SDL_ttf.h>

#include <tuple>

#include <QStyleOptionButton>
#include <QStylePainter>

#include "SDLPaintEngine.h"
#include "Common.h"

template<int TId, typename TUserState>
struct ButtonState
{
	STATE_PROPERTY(VisualState, state);
	STATE_PROPERTY(SDL_Point, size);
	STATE_PROPERTY(SDL_Point, position);
	STATE_PROPERTY(Callback<TUserState>, on_clicked);
	STATE_PROPERTY(const char *, text);
};

template<Operation TOperation, typename TContext, typename ...TProperties>
struct ButtonLogic
{
};

template<typename TContext, typename ...TProperties>
struct ButtonLogic<Operation::Initialize, TContext, TProperties...>
{
	static auto invoke(const TContext &context, TProperties &...)
	{
		return context_prepend(ButtonState<Level<TContext>, UserState<TContext>>(), context);
	}
};

template<typename TContext, typename ...TProperties>
struct ButtonLogic<Operation::Update, TContext, TProperties...>
{
	static auto invoke(const TContext &context, const TProperties &...properties)
	{
		const auto &button = std::get<ButtonState<Level<TContext>, UserState<TContext>>>(context.state);

		return handle_events(
			calculate_state(
				repack(context,
					apply_properties(button, properties...)
				)
			)
		);
	}

	static auto handle_events(const TContext &context)
	{
		const auto &root = std::get<RootState>(context.state);
		const auto &user = std::get<UserState<TContext>>(context.state);
		const auto &button = std::get<ButtonState<Level<TContext>, UserState<TContext>>>(context.state);

		if (button.on_clicked == nullptr)
		{
			return context;
		}

		if (root.event.button.type != SDL_MOUSEBUTTONDOWN)
		{
			return context;
		}

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (SDL_PointInRect(&point, &rect))
		{
			return repack(context, button.on_clicked(user));
		}

		return context;
	}

	static auto calculate_state(const TContext &context)
	{
		const auto &root = std::get<RootState>(context.state);
		const auto &button = std::get<ButtonState<Level<TContext>, UserState<TContext>>>(context.state);

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (!SDL_PointInRect(&point, &rect))
		{
			return repack(context, button.with_state(VisualState::Normal));
		}

		if (root.event.button.type == SDL_MOUSEBUTTONDOWN)
		{
			return repack(context, button.with_state(VisualState::Pressed));
		}

		return repack(context, button.with_state(VisualState::Highlight));
	}
};

template<typename TContext, typename ...TProperties>
struct ButtonLogic<Operation::Draw, TContext, TProperties...>
{
	static auto invoke(const TContext &context, TProperties &...)
	{
		const auto &button = std::get<ButtonState<Level<TContext>, UserState<TContext>>>(context.state);
		const auto &root = std::get<RootState>(context.state);

		SDLPaintDevice device(root.surface);

		QPainter painter(&device);

		QStyleOptionButton option;
		option.rect = QRect(button.position.x, button.position.y, button.size.x, button.size.y);
		option.text = button.text;
		option.state = QStyle::State_Active | QStyle::State_Enabled;

		if (button.state == VisualState::Highlight)
		{
			option.state |= QStyle::State_MouseOver;
		}

		if (button.state == VisualState::Pressed)
		{
			option.state |= QStyle::State_Sunken;
		}

		QApplication::style()->drawControl(QStyle::CE_PushButton, &option, &painter, nullptr);

		return context;
	}
};

template<typename TContext, typename ...TProperties>
auto Button(const TContext &context, TProperties ...properties)
{
	return ButtonLogic<Op<TContext>, decltype(level_up(context)), TProperties...>::invoke(level_up(context), properties...);
}

#endif // BUTTON_H
