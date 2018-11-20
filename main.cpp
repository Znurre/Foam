#include <SDL.h>

#include <tuple>

#include "Repack.h"
#include "Properties.h"
#include "Button.h"

struct State
{
	State with_counter(int counter) const
	{
		State copy(*this);
		copy.counter = counter;

		return copy;
	}

	int counter;
};

template<typename TState, typename ...TChild>
auto Horizontal(TState &state, const TChild &...)
{
	return state;
}

template<typename TState, typename ...TChild>
auto Vertical(TState state, const TChild &...)
{
	return state;
}

State increment_counter(const State &state)
{
	return state.with_counter(state.counter + 1);
}

State decrement_counter(const State &state)
{
	return state.with_counter(state.counter - 1);
}

template<Operation TOperation, typename TState>
auto layout(const TState &state)
{
	return
		Button<1, TOperation>(
			Button<2, TOperation>(state
				, position = SDL_Point { 120, 100 - std::get<State>(state).counter * 5 }
				, size = SDL_Point { 100, 30 }
				, on_clicked = &decrement_counter
			)
			, position = SDL_Point { 10, 100 + std::get<State>(state).counter * 5 }
			, size = SDL_Point { 100, 30 }
			, on_clicked = &increment_counter
		);
}

template<typename TState>
auto run(const TState &state) -> decltype(layout<Operation::Update>(state))
{
	SDL_Event event;

	const auto &root = std::get<RootState>(state);

	SDL_UpdateWindowSurface(root.window);

	SDL_FillRect(root.surface, nullptr, 0x00000000);
	SDL_WaitEvent(&event);

	return run(
		layout<Operation::Draw>(
			layout<Operation::Update>(
				repack(state, root.with_event(event))
			)
		)
	);
}

int main(int, char **)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	auto window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
	auto surface = SDL_GetWindowSurface(window);

	State state;

	RootState root;
	root.window = window;
	root.surface = surface;

	const auto tuple = std::make_tuple(root, state);

	run(layout<Operation::Initialize>(tuple));

	return 0;
}
