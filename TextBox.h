#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Item.h"

template<int TId, typename TUserState>
struct TextBoxState
{
	STATE_PROPERTY(glm::vec2, size)
	STATE_PROPERTY(glm::vec2, position)
	STATE_PROPERTY(VisualState, state)
};

template<Operation TOperation>
struct TextBoxLogic
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		return context;
	}
};

template<>
struct TextBoxLogic<Operation::Initialize>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &text_box = TextBoxState<get_level_v<TContext>, get_user_state_t<TContext>>();
		const auto &new_context = context_prepend(text_box, context);

		return expand_templates(new_context
			, get_style_t<TContext>::TextBoxStyle::Normal::layout(text_box)
			, get_style_t<TContext>::TextBoxStyle::Hover::layout(text_box)
			, get_style_t<TContext>::TextBoxStyle::Focused::layout(text_box)
			);
	}
};

template<>
struct TextBoxLogic<Operation::Update>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &properties)
	{
		const auto &new_context =
			handle_focus(
				calculate_state(
					repack(context,
						apply_properties(properties,
							read_control_state<TextBoxState>(context)
						)
					)
				)
			);

		const auto &text_box = read_control_state<TextBoxState>(new_context);

		return expand_templates(new_context
			, get_style_t<TContext>::TextBoxStyle::Normal::layout(text_box)
			, get_style_t<TContext>::TextBoxStyle::Hover::layout(text_box)
			, get_style_t<TContext>::TextBoxStyle::Focused::layout(text_box)
			);
	}

	template<typename TContext>
	static TContext handle_focus(const TContext &context)
	{
		const auto &root = read_root_state(context);
		const auto &text_box = read_control_state<TextBoxState>(context);

		const SDL_Rect rect = { (int)text_box.position.x, (int)text_box.position.y, (int)text_box.size.x, (int)text_box.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (!SDL_PointInRect(&point, &rect))
		{
			return context;
		}

		if (root.event.button.type == SDL_MOUSEBUTTONDOWN)
		{
			return repack(context, root.with_focused(get_level_v<TContext>));
		}

		return context;
	}

	template<typename TContext>
	static TContext calculate_state(const TContext &context)
	{
		const auto &root = read_root_state(context);
		const auto &text_box = read_control_state<TextBoxState>(context);

		const SDL_Rect rect = { (int)text_box.position.x, (int)text_box.position.y, (int)text_box.size.x, (int)text_box.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (!SDL_PointInRect(&point, &rect))
		{
			return repack(context, text_box.with_state(VisualState::Normal));
		}

		return repack(context, text_box.with_state(VisualState::Hover));
	}
};

template<>
struct TextBoxLogic<Operation::Draw>
{
	template<typename TContext, typename ...TProperties>
	static auto invoke(const TContext &context, const std::tuple<TProperties...> &)
	{
		const auto &root = read_root_state(context);
		const auto &text_box = read_control_state<TextBoxState>(context);

		if (root.focused == get_level_v<TContext>)
		{
			return expand_templates(context
				, get_style_t<TContext>::TextBoxStyle::Normal::layout(text_box) | skip
				, get_style_t<TContext>::TextBoxStyle::Hover::layout(text_box) | skip
				, get_style_t<TContext>::TextBoxStyle::Focused::layout(text_box)
				);
		}

		if (text_box.state == VisualState::Hover)
		{
			return expand_templates(context
				, get_style_t<TContext>::TextBoxStyle::Normal::layout(text_box) | skip
				, get_style_t<TContext>::TextBoxStyle::Hover::layout(text_box)
				, get_style_t<TContext>::TextBoxStyle::Focused::layout(text_box) | skip
				);
		}

		return expand_templates(context
			, get_style_t<TContext>::TextBoxStyle::Normal::layout(text_box)
			, get_style_t<TContext>::TextBoxStyle::Hover::layout(text_box) | skip
			, get_style_t<TContext>::TextBoxStyle::Focused::layout(text_box) | skip
			);
	}
};

template<typename ...TParameters>
struct TextBox : public Item<TextBoxLogic, TParameters...>
{
	TextBox(const TParameters &...parameters)
		: Item<TextBoxLogic, TParameters...>(parameters...)
	{
	}
};

#endif // TEXTBOX_H
