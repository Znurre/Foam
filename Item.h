#ifndef ITEM_H
#define ITEM_H

#include <tuple>

#include "Context.h"
#include "Properties.h"

template<typename TContext>
auto build_children(const TContext &context)
{
	return context;
}

template<typename TContext, typename TChild>
auto build_children(const TContext &context, const TChild &child)
{
	return child.build(context);
}

template<typename TContext, typename TChild, typename ...TChildren>
auto build_children(const TContext &context, const TChild &child, const TChildren&... children)
{
	return build_children(child.build(context), children...);
}

template<typename TContext, typename TTuple, std::size_t ...TIndex>
auto expand_children(const TContext &context, const TTuple &tuple, std::index_sequence<TIndex...>)
{
	return build_children(context, std::get<TIndex>(tuple)...);
}

template<typename TContext, typename TTuple>
auto expand_children(const TContext &context, const TTuple &tuple)
{
	return expand_children(context, tuple, std::make_index_sequence<std::tuple_size_v<TTuple>>());
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

struct Object
{
};

template<template<Operation> class TLogic, typename ...TParameters>
struct Item : public Object
{
	Item(const TParameters &...parameters)
		: m_parameters(parameters...)
	{
	}

	template<typename T>
	using PropertyTypePredicate = std::is_base_of<Property, T>;

	template<typename T>
	using ChildrenTypePredicate = std::is_base_of<Object, T>;

	template<typename TContext>
	auto build(const TContext &context) const
	{
		const auto properties = tuple_filter<PropertyTypePredicate>(m_parameters);
		const auto children = tuple_filter<ChildrenTypePredicate>(m_parameters);

		return expand_children(TLogic<get_operation_v<TContext>>::invoke(level_up(context), properties), children);
	}

	std::tuple<TParameters...> m_parameters;
};

#endif // ITEM_H
