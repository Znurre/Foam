#ifndef ITEM_H
#define ITEM_H

#include "Context.h"
#include "Properties.h"
#include "Common.h"

template<typename TContext>
auto build_children(TContext &&context)
{
	return context;
}

template<typename TContext, typename TChild>
auto build_children(TContext &&context, const TChild &child)
{
	return child.build(context);
}

template<typename TContext, typename TChild, typename ...TChildren>
auto build_children(TContext &&context, const TChild &child, const TChildren&... children)
{
	return build_children(child.build(context), children...);
}

template<typename TContext, typename TTuple, std::size_t ...TIndex>
auto expand_children(TContext &&context, const TTuple &tuple, std::index_sequence<TIndex...>)
{
	return build_children(context, std::get<TIndex>(tuple)...);
}

template<typename TContext, typename TTuple>
auto expand_children(TContext &&context, const TTuple &tuple)
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

	template<typename T>
	using PropertyTypePredicate = std::is_base_of<Property, T>;

	template<typename T>
	using ChildrenTypePredicate = std::is_base_of<Object, T>;

	template<typename TContext>
	auto build(const TContext &context) const
	{
		const auto &properties = tuple_filter<PropertyTypePredicate>(m_parameters);
		const auto &children = tuple_filter<ChildrenTypePredicate>(m_parameters);

		return expand_children(TLogic<get_operation_v<TContext>>::invoke(level_up(context), properties), children);
	}

	std::tuple<TParameters...> m_parameters;
};

#endif // ITEM_H
