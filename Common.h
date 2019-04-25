#ifndef COMMON_H
#define COMMON_H

#include <GL/glew.h>

#include <SDL.h>

#include <glm/mat4x4.hpp>

#include "Context.h"
#include "Properties.h"

struct RootState
{
	RootState()
		: focused(-1)
	{
	}

	RootState with_event(const SDL_Event &event) const
	{
		RootState copy(*this);
		copy.event = event;

		return copy;
	}

	RootState with_focused(int id) const
	{
		RootState copy(*this);
		copy.focused = id;

		return copy;
	}

	SDL_Window *window;
	SDL_GLContext context;

	SDL_Event event;

	int focused;

	GLuint vbo;
	GLuint ibo;
	GLuint vao;
};

struct DrawCommand
{
	glm::mat3 matrix;

	uint color;
};

struct DrawableControl
{
};

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
TState apply_properties(const TTuple &tuple, const TState &state)
{
	return apply_properties(state, tuple, std::make_index_sequence<std::tuple_size_v<TTuple>>());
}

template<template<int, typename> class TControlState, typename TContext>
auto read_control_state(const TContext &context)
{
	return std::get<TControlState<get_level_v<TContext>, get_user_state_t<TContext>>>(context.state);
}

template<typename TState>
auto read_user_state(const TState &state)
{
	return std::get<get_user_state_t<TState>>(state);
}

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
auto read_user_state(const Context<TOperation, TStyle, TLevel, TState> &context)
{
	return std::get<get_user_state_t<TState>>(context.state);
}

template<typename TContext>
auto read_root_state(const TContext &context)
{
	return std::get<RootState>(context.state);
}

template<typename TupleOfIntegralConstant>
struct as_sequence;

template<typename ...Ts>
struct as_sequence<std::tuple<Ts...>>
{
	using type = std::index_sequence<Ts::value...>;
};

template<typename ...Ts>
using as_sequence_t = typename as_sequence<Ts...>::type;

template<template <typename> class TPredicate, typename TTuple, typename TSequence>
struct make_filtered_sequence;

template<template <typename> class TPredicate, typename TTuple, std::size_t ...TIndex>
struct make_filtered_sequence<TPredicate, TTuple, std::index_sequence<TIndex...>>
{
	using type = as_sequence_t<decltype(std::tuple_cat(
		std::conditional_t<TPredicate<std::tuple_element_t<TIndex, TTuple>>::value
			, std::tuple<std::integral_constant<std::size_t, TIndex>>
			, std::tuple<>>{}...))>;
};

template<template <typename> class TPredicate, typename TTuple, typename TSequence>
using make_filtered_sequence_t = typename make_filtered_sequence<TPredicate, TTuple, TSequence>::type;

template <typename TTuple, std::size_t ...TIndex>
auto tuple_filter(const TTuple &tuple, std::index_sequence<TIndex...>) -> std::tuple<std::tuple_element_t<TIndex, TTuple>...>
{
	return { std::get<TIndex>(tuple)... };
}

template <template <typename> class TPredicate, typename TTuple>
auto tuple_filter(const TTuple &tuple)
{
	using filtered_sequence = make_filtered_sequence_t<TPredicate, TTuple, std::make_index_sequence<std::tuple_size<TTuple>::value>>;

	return tuple_filter(tuple, filtered_sequence());
}

enum class VisualState
{
	Normal = 0,
	Hover,
	Pressed,
	Disabled
};

#endif // COMMON_H
