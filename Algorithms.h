#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <memory>

template<typename InputIt, typename OutputIt, typename T, typename BinaryOperation>
OutputIt fold_transform(InputIt first, InputIt last, OutputIt d_first, T init, BinaryOperation binary_op)
{
	if (first == last)
	{
		return d_first;
	}

	init = binary_op(*first, std::move(init));

	*d_first++ = init;

	return fold_transform(first + 1, last, d_first, init, binary_op);
}

template<typename InputIt, typename OutputIt, typename BinaryOperation>
OutputIt fold_transform(InputIt first, InputIt last, OutputIt d_first, BinaryOperation binary_op)
{
	std::decay_t<typename OutputIt::container_type::value_type> init;

	return fold_transform(first, last, d_first, init, binary_op);
}

#endif // ALGORITHMS_H
