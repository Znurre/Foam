#ifndef BUTTON_H
#define BUTTON_H

#include <QApplication>
#include <SDL.h>

#include <tuple>

#include <QStyleOptionButton>
#include <QStylePainter>

#include "SDLPaintEngine.h"
#include "Common.h"
#include "Item.h"

template<int TId, typename TUserState>
struct ButtonState
{
	STATE_PROPERTY(VisualState, state);
	STATE_PROPERTY(SDL_Point, size);
	STATE_PROPERTY(SDL_Point, position);
	STATE_PROPERTY(Callback<TUserState>, on_clicked);
	STATE_PROPERTY(const char *, text);
};

template<Operation TOperation>
struct ButtonLogic
{
};

template<>
struct ButtonLogic<Operation::Initialize>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context_prepend(ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>(), context);
	}
};

template<>
struct ButtonLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &button = std::get<ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);

		return handle_events(
			calculate_state(
				repack(context,
					apply_properties(button, properties)
				)
			)
		);
	}

	template<typename TContext>
	static auto handle_events(const TContext &context)
	{
		const auto &root = std::get<RootState>(context.state);
		const auto &user = std::get<get_user_state_t<TContext>>(context.state);
		const auto &button = std::get<ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);

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

	template<typename TContext>
	static auto calculate_state(const TContext &context)
	{
		const auto &root = std::get<RootState>(context.state);
		const auto &button = std::get<ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);

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

template<>
struct ButtonLogic<Operation::Draw>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &button = std::get<ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);
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

template<typename ...TParameters>
struct Button : public Item<ButtonLogic, TParameters...>
{
	Button(const TParameters &...parameters)
		: Item<ButtonLogic, TParameters...>(parameters...)
	{
	}
};

#endif // BUTTON_H
