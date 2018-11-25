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

template<Operation TOperation, typename TState, int TId, typename ...TProperties>
struct ButtonLogic
{
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Initialize, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		return tuple_prepend(ButtonState<TId, UserState<TState>>(), state);
	}
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Update, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, const TProperties &...properties)
	{
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);

		return handle_events(
			calculate_state(
				repack(state,
					apply_properties(button, properties...)
				)
			)
		);
	}

	static auto handle_events(const TState &state)
	{
		const auto &root = std::get<RootState>(state);
		const auto &user = std::get<UserState<TState>>(state);
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);

		if (button.on_clicked == nullptr)
		{
			return state;
		}

		if (root.event.button.type != SDL_MOUSEBUTTONDOWN)
		{
			return state;
		}

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (SDL_PointInRect(&point, &rect))
		{
			return repack(state, button.on_clicked(user));
		}

		return state;
	}

	static auto calculate_state(const TState &state)
	{
		const auto &root = std::get<RootState>(state);
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (!SDL_PointInRect(&point, &rect))
		{
			return repack(state, button.with_state(VisualState::Normal));
		}

		if (root.event.button.type == SDL_MOUSEBUTTONDOWN)
		{
			return repack(state, button.with_state(VisualState::Pressed));
		}

		return repack(state, button.with_state(VisualState::Highlight));
	}
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Draw, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);
		const auto &root = std::get<RootState>(state);

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

		return state;
	}
};

template<int TId, Operation TOperation, typename TState, typename ...TProperties>
auto Button(const TState &state, TProperties ...properties)
{
	return ButtonLogic<TOperation, TState, TId, TProperties...>::invoke(state, properties...);
}

#endif // BUTTON_H
