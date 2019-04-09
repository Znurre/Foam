#include <QApplication>

#include <SDL.h>

#include <tuple>

#include "Repack.h"
#include "Properties.h"
#include "Button.h"
#include "Rectangle.h"
#include "MouseArea.h"

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

//template<typename TState, typename ...TProperties>
//auto ButtonComposite(const TState &state, const TProperties &...properties)
//{
//	struct ButtonProperties
//	{
//		STATE_PROPERTY(SDL_Point, size);
//		STATE_PROPERTY(SDL_Point, position);
//	};

//	const auto &values = apply_properties(ButtonProperties(), std::make_tuple(properties...));

//	return
//		MouseArea(
//			Rectangle(
//				Rectangle(state
//					, size = values.size
//					, position = values.position
//					, color = SDL_Color { 0x0, 0x0, 0x0, 0xFF }
//				)
//				, size = SDL_Point { values.size.x - 2, values.size.y - 2 }
//				, position = SDL_Point { values.position.x + 1, values.position.y + 1 }
//				, color = SDL_Color { 0xFF, 0xFF, 0xFF, 0xFF }
//			)
//			, size = values.size
//			, position = values.position
//		);
//}


template<typename TState>
auto layout(const TState &state)
{
	return Rectangle
	{
		Button
		{
			position = SDL_Point { 120, 100 - state.counter * 5 },
			size = SDL_Point { 100, 30 },
			on_clicked = &decrement_counter,
			text = state.get_button_text()
		},

		Button
		{
			position = SDL_Point { 10, 100 + state.counter * 5 },
			size = SDL_Point { 100, 30 },
			on_clicked = &increment_counter,
			text = state.get_button_text()
		},

		position = SDL_Point { 100, 10 },
		size = SDL_Point { 50, 50 },
		color = SDL_Color { 0xFF, 0x00, 0x00, 0xFF },
	};
}

template <Operation TOperation, typename TState>
auto layout(const TState &state)
{
	const auto &user = read_user_state(state);
	const auto &root = layout(user);
	const auto &context = make_context<TOperation>(state);
	const auto &result = root.build(context);

	return strip_context(result);
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

	auto window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	auto surface = SDL_GetWindowSurface(window);

	State state;

	RootState root;
	root.window = window;
	root.surface = surface;

	const auto tuple = std::make_tuple(root, state);

	run(layout<Operation::Initialize>(tuple));

	return 0;
}
