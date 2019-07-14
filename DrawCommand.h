#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include "Properties.h"
#include "Vector.h"

#include <vector>
#include <tuple>
#include <numeric>

#include <glm/mat4x4.hpp>

struct DrawCommand
{
	DrawCommand()
		: matrix(1.0f)
		, uv(0.0f)
		, color(0)
	{
	}

	STATE_PROPERTY(glm::mat3, matrix)
	STATE_PROPERTY(glm::vec4, uv)
	STATE_PROPERTY(uint, color)
};

uint32_t create_hash(uint32_t offset, const DrawCommand &value)
{
	const auto begin = reinterpret_cast<const uint8_t *>(&value);
	const auto end = begin + sizeof(value);

	return std::accumulate(begin, end, offset, [](uint32_t hash, uint8_t octet)
	{
		return (hash ^ octet) * 16777619;
	});
};

template<typename TTuple, size_t TIndex>
struct compute_hash_impl
{
	static uint32_t value(const TTuple &tuple)
	{
		const auto &hash = compute_hash_impl<TTuple, TIndex - 1>::value(tuple);
		const auto &control = std::get<TIndex - 1>(tuple);

		return std::accumulate(std::begin(control.draw_commands)
			, std::end(control.draw_commands)
			, hash
			, &create_hash
			);
	}
};

template<typename TTuple>
struct compute_hash_impl<TTuple, 0>
{
	static uint32_t value(const TTuple &)
	{
		return 2166136261u;
	}
};

template<typename TTuple>
uint32_t compute_hash(const TTuple &tuple)
{
	return compute_hash_impl<TTuple, std::tuple_size_v<TTuple>>::value(tuple);
}

template<typename TControl>
auto get_draw_command(const TControl &control)
{
	return control.draw_commands;
}

template<typename TTuple, std::size_t ...TIndex>
immutable_vector<DrawCommand> extract_draw_commands(const TTuple &tuple, std::index_sequence<TIndex...>)
{
	return { get_draw_command(std::get<std::tuple_size_v<TTuple> - (TIndex + 1)>(tuple))... };
}

template<typename TTuple>
auto extract_draw_commands(const TTuple &tuple)
{
	return extract_draw_commands(tuple, std::make_index_sequence<std::tuple_size_v<TTuple>>());
}

#endif // DRAWCOMMAND_H
