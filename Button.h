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
#include "Style.h"

template<int TId, typename TUserState>
struct ButtonState
{
	STATE_PROPERTY(VisualState, state)
	STATE_PROPERTY(SDL_Point, size)
	STATE_PROPERTY(SDL_Point, position)
	STATE_PROPERTY(Callback<TUserState>, on_clicked)
	STATE_PROPERTY(const char *, text)
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
		const auto &button = ButtonState<get_level_v<TContext>, get_user_state_t<TContext>>();
		const auto &new_context = context_prepend(button, context);

		return expand_templates(new_context
			, get_style_t<TContext>::ButtonStyle::Normal::layout(button)
			, get_style_t<TContext>::ButtonStyle::Hover::layout(button)
			, get_style_t<TContext>::ButtonStyle::Pressed::layout(button)
			);
	}
};

template<>
struct ButtonLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &new_context =
			handle_events(
				calculate_state(
					repack(context,
						apply_properties(properties,
							read_control_state<ButtonState>(context)
						)
					)
				)
			);

		const auto &button = read_control_state<ButtonState>(new_context);

		return expand_templates(new_context
			, get_style_t<TContext>::ButtonStyle::Normal::layout(button)
			, get_style_t<TContext>::ButtonStyle::Hover::layout(button)
			, get_style_t<TContext>::ButtonStyle::Pressed::layout(button)
			);
	}

	template<typename TContext>
	static auto handle_events(const TContext &context)
	{
		const auto &root = std::get<RootState>(context.state);
		const auto &user = read_user_state(context);
		const auto &button = read_control_state<ButtonState>(context);

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
		const auto &button = read_control_state<ButtonState>(context);

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
		const auto &button = read_control_state<ButtonState>(context);

		if (button.state == VisualState::Highlight)
		{
			return expand_templates(context
				, get_style_t<TContext>::ButtonStyle::Normal::layout(button) | skip
				, get_style_t<TContext>::ButtonStyle::Hover::layout(button)
				, get_style_t<TContext>::ButtonStyle::Pressed::layout(button) | skip
				);
		}

		if (button.state == VisualState::Pressed)
		{
			return expand_templates(context
				, get_style_t<TContext>::ButtonStyle::Normal::layout(button) | skip
				, get_style_t<TContext>::ButtonStyle::Hover::layout(button) | skip
				, get_style_t<TContext>::ButtonStyle::Pressed::layout(button)
				);
		}

		return expand_templates(context
			, get_style_t<TContext>::ButtonStyle::Normal::layout(button)
			, get_style_t<TContext>::ButtonStyle::Hover::layout(button) | skip
			, get_style_t<TContext>::ButtonStyle::Pressed::layout(button) | skip
			);
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
