#ifndef COMMON_H
#define COMMON_H

#include <SDL.h>

#include "Context.h"

template<typename TUserState>
using Callback = TUserState (*)(const TUserState &);

template<typename TState>
TState expand_properties(const TState &state)
{
	return state;
}

template<typename TState, typename TProperty>
TState expand_properties(const TState &state, const TProperty &property)
{
	return property.apply(state);
}

template<typename TState, typename TProperty, typename ...TProperties>
TState expand_properties(const TState &state, const TProperty &property, const TProperties&... properties)
{
	return expand_properties(property.apply(state), properties...);
}

template<typename TState, typename TTuple, std::size_t ...TIndex>
TState apply_properties(const TState &state, const TTuple &tuple, std::index_sequence<TIndex...>)
{
	return expand_properties(state, std::get<TIndex>(tuple)...);
}

template<typename TState, typename TTuple>
TState apply_properties(const TState &state, const TTuple &tuple)
{
	return apply_properties(state, tuple, std::make_index_sequence<std::tuple_size_v<TTuple>>());
}

template<typename TState>
auto read_user_state(const TState &state)
{
	return std::get<get_user_state_t<TState>>(state);
}

template<typename TState>
auto parent(const TState &state)
{
	return std::get<0>(state);
}

enum class VisualState
{
	Normal = 0,
	Highlight,
	Pressed,
	Disabled
};

struct RootState
{
	RootState with_event(const SDL_Event &event) const
	{
		RootState copy(*this);
		copy.event = event;

		return copy;
	}

	SDL_Window *window;
	SDL_Surface *surface;

	SDL_Event event;
};

#endif // COMMON_H
