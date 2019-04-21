#ifndef STYLE_H
#define STYLE_H

#include "Context.h"

template<typename TNormal, typename THover, typename TPressed>
struct ButtonStyle
{
	typedef TNormal Normal;
	typedef THover Hover;
	typedef TPressed Pressed;
};

template<typename TNormal, typename THover, typename TFocused>
struct TextBoxStyle
{
	typedef TNormal Normal;
	typedef THover Hover;
	typedef TFocused Focused;
};

template<typename TButtonStyle, typename TTextBoxStyle>
struct Style
{
	typedef TButtonStyle ButtonStyle;
	typedef TTextBoxStyle TextBoxStyle;
};

template<typename TTemplate>
struct skip_template
{
	skip_template(const TTemplate &subject)
		: subject(subject)
	{
	}

	template<typename TContext>
	auto build(const TContext &context) const
	{
		const auto &noop_context = change_operation<Operation::Noop>(context);
		const auto &result = subject.build(noop_context);

		return change_operation<get_operation_v<TContext>>(result);
	}

	TTemplate subject;
};

struct skip_template_decorator
{
};

template<class TTemplate>
auto operator|(const TTemplate &t, const skip_template_decorator &)
{
	return skip_template<TTemplate>(t);
}

constexpr skip_template_decorator skip;

template<typename TContext, typename TTemplates>
auto expand_templates(const TContext &context, const TTemplates &t)
{
	return t.build(context);
}

template<typename TContext, typename TTemplate, typename ...TTemplates>
auto expand_templates(const TContext &context, const TTemplate &t, const TTemplates &... templates)
{
	return expand_templates(expand_templates(context, templates...), t);
}

#endif // STYLE_H
