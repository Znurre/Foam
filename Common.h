#ifndef COMMON_H
#define COMMON_H

#include <SDL.h>
#include <SDL_ttf.h>

template<typename TState>
using UserState = typename std::tuple_element<std::tuple_size<TState>::value - 1, TState>::type;

template<typename TUserState>
using Callback = TUserState (*)(const TUserState &state);

template<typename TState, typename TProperty>
TState apply_properties(const TState &state, const TProperty &property)
{
	return property.apply(state);
}

template<typename TState, typename TProperty, typename ...TProperties>
TState apply_properties(const TState &state, const TProperty &property, const TProperties&... properties)
{
	return apply_properties(property.apply(state), properties...);
}

enum class VisualState
{
	Normal = 0,
	Highlight,
	Disabled
};

enum class Operation
{
	Initialize,
	Update,
	Draw
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

	TTF_Font *font;

	SDL_Event event;
};

#endif // COMMON_H
