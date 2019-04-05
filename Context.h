#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>

template<int TLevel, typename TState>
struct Context
{
	TState state;
};

template<typename TState>
Context<0, TState> make_context(const TState &state)
{
	return { state };
}

template<int TLevel, typename TState>
Context<0, TState> reset_level(const Context<TLevel, TState> &context)
{
	return { context.state };
}

template<int TLevel, typename TState, template<int, typename> class TContext>
Context<TLevel + 1, TState> level_up(const TContext<TLevel, TState> &context)
{
	return { context.state };
}

template<int TLevel, typename TState, template<int, typename> class TContext>
TState strip_level(const TContext<TLevel, TState> &context)
{
	return context.state;
}

template<typename T>
struct get_level;

template<int TLevel, typename TState>
struct get_level<Context<TLevel, TState>>
{
	static const int value = TLevel;
};

template<typename T>
constexpr int Level = get_level<T>::value;

template<typename TState>
struct get_user_state;

template<int TLevel, typename TState>
struct get_user_state<Context<TLevel, TState>> : std::tuple_element<std::tuple_size<TState>::value - 1, TState>
{
};

template<typename TState>
using UserState = typename get_user_state<TState>::type;

#endif // CONTEXT_H
