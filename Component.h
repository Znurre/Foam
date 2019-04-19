#ifndef COMPONENT_H
#define COMPONENT_H

#include "Item.h"

template<typename T, typename ...TParameters>
struct Component : public Object
{
	Component(const TParameters &...parameters)
		: m_parameters(parameters...)
	{
	}

	template<typename TContext>
	auto build(const TContext &context) const
	{
		const auto &state = read_user_state(context);
		const auto &item = T::layout(state, m_parameters);

		return item.build(context);
	}

	std::tuple<TParameters...> m_parameters;
};

#endif // COMPONENT_H
