#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>

enum class Operation
{
	Initialize,
	Update,
	Draw
};

template<Operation TOperation, int TLevel, typename TState>
struct Context
{
	TState state;
};

template<Operation TOperation, typename TState>
Context<TOperation, 0, TState> make_context(const TState &state)
{
	return { state };
}

template<Operation TOperation, int TLevel, typename TState>
Context<TOperation, TLevel + 1, TState> level_up(const Context<TOperation, TLevel, TState> &context)
{
	return { context.state };
}

template<Operation TOperation, int TLevel, typename TState>
TState strip_context(const Context<TOperation, TLevel, TState> &context)
{
	return context.state;
}

template<typename T>
struct get_operation;

template<Operation TOperation, int TLevel, typename TState>
struct get_operation<Context<TOperation, TLevel, TState>>
{
	static const Operation value = TOperation;
};

template<typename T>
constexpr Operation Op = get_operation<T>::value;

template<typename T>
struct get_level;

template<Operation TOperation, int TLevel, typename TState>
struct get_level<Context<TOperation, TLevel, TState>>
{
	static const int value = TLevel;
};

template<typename T>
constexpr int Level = get_level<T>::value;

template<typename TState>
struct get_user_state;

template<Operation TOperation, int TLevel, typename TState>
struct get_user_state<Context<TOperation, TLevel, TState>> : std::tuple_element<std::tuple_size<TState>::value - 1, TState>
{
};

template<typename TState>
using UserState = typename get_user_state<TState>::type;

#endif // CONTEXT_H
