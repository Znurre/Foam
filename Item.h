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

	static auto filter_properties(TParameters...)
	{
		return std::tuple_cat(std::conditional_t<std::is_base_of_v<Property, TParameters>
			, std::tuple<TParameters>
			, std::tuple<>>()...
			);
	}

	static auto filter_children(TParameters...)
	{
		return std::tuple_cat(std::conditional_t<std::is_base_of_v<Object, TParameters>
			, std::tuple<TParameters>
			, std::tuple<>>()...
			);
	}

	template<typename TContext>
	auto build(const TContext &context) const
	{
		const auto properties = std::apply(&filter_properties, m_parameters);
		const auto children = std::apply(&filter_children, m_parameters);

		return expand_children(TLogic<Op<TContext>>::invoke(level_up(context), properties), children);
	}

	std::tuple<TParameters...> m_parameters;
};

#endif // ITEM_H
