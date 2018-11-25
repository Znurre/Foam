#include <QApplication>

#include <SDL.h>
#include <SDL_ttf.h>

#include <tuple>

#include "Repack.h"
#include "Properties.h"
#include "Button.h"

struct State
{
	State()
		: counter(0)
	{
	}

	State with_counter(int counter) const
	{
		State copy(*this);
		copy.counter = counter;

		return copy;
	}

	const char *get_button_text() const
	{
		static char string[100];

		sprintf(string, "Value: %d", counter);

		return string;
	}

	int counter;
};

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
				, text = std::get<State>(state).get_button_text()
			)
			, position = SDL_Point { 10, 100 + std::get<State>(state).counter * 5 }
			, size = SDL_Point { 100, 30 }
			, on_clicked = &increment_counter
			, text = std::get<State>(state).get_button_text()
		);
}

template<typename TState>
auto run(const TState &state) -> decltype(layout<Operation::Update>(state))
{
	SDL_Event event;

	const auto &root = std::get<RootState>(state);

	SDL_UpdateWindowSurface(root.window);

	SDL_WaitEvent(&event);
	SDL_FillRect(root.surface, nullptr, 0xFFeff0f1);

	return run(
		layout<Operation::Draw>(
			layout<Operation::Update>(
				repack(state, root.with_event(event))
			)
		)
	);
}

int main(int argc, char **argv)
{
	QApplication application(argc, argv);

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	auto window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	auto surface = SDL_GetWindowSurface(window);
	auto font = TTF_OpenFont("Liberation Sans, Regular.ttf", 12);

	State state;

	RootState root;
	root.window = window;
	root.surface = surface;
	root.font = font;

	const auto tuple = std::make_tuple(root, state);

	run(layout<Operation::Initialize>(tuple));

	return 0;
}
