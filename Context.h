#ifndef CONTEXT_H
#define CONTEXT_H

#include <tuple>

enum class Operation
{
	Noop,
	Initialize,
	Update,
	Draw
};

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
struct Context
{
	TState state;
};

template<Operation TOperation, typename TStyle, typename TState>
Context<TOperation, TStyle, 0, TState> make_context(const TState &state)
{
	return { state };
}

template<Operation TNewOperation, Operation TOperation, typename TStyle, int TLevel, typename TState>
Context<TNewOperation, TStyle, TLevel, TState> change_operation(const Context<TOperation, TStyle, TLevel, TState> &context)
{
	return { context.state };
}

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
Context<TOperation, TStyle, TLevel + 1, TState> level_up(const Context<TOperation, TStyle, TLevel, TState> &context)
{
	return { context.state };
}

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
TState strip_context(const Context<TOperation, TStyle, TLevel, TState> &context)
{
	return context.state;
}

template<typename T>
struct get_operation;

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
struct get_operation<Context<TOperation, TStyle, TLevel, TState>>
{
	static const Operation value = TOperation;
};

template<typename T>
constexpr Operation get_operation_v = get_operation<T>::value;

template<typename T>
struct get_level;

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
struct get_level<Context<TOperation, TStyle, TLevel, TState>>
{
	static const int value = TLevel;
};

template<typename T>
constexpr int get_level_v = get_level<T>::value;

template<typename TState>
struct get_user_state  : std::tuple_element<std::tuple_size<TState>::value - 1, TState>
{
};

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
struct get_user_state<Context<TOperation, TStyle, TLevel, TState>> : std::tuple_element<std::tuple_size<TState>::value - 1, TState>
{
};

template<typename TContext>
using get_user_state_t = typename get_user_state<TContext>::type;

template<typename T>
struct get_style;

template<Operation TOperation, typename TStyle, int TLevel, typename TState>
struct get_style<Context<TOperation, TStyle, TLevel, TState>>
{
	typedef TStyle type;
};

template<typename T>
using get_style_t = typename get_style<T>::type;

#endif // CONTEXT_H
