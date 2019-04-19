#include <QApplication>

#include <tuple>

#include "Application.h"
#include "Properties.h"
#include "Button.h"
#include "Rectangle.h"
#include "MouseArea.h"
#include "Component.h"
#include "DefaultStyle.h"

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

template<typename ...TParameters>
struct ButtonComposite : public Component<ButtonComposite<TParameters...>, TParameters...>
{
	ButtonComposite(const TParameters &...parameters)
		: Component<ButtonComposite, TParameters...>(parameters...)
	{
	}

	template<typename TState>
	static auto layout(const TState &, const std::tuple<TParameters...> &parameters)
	{
		struct ButtonProperties
		{
			STATE_PROPERTY(SDL_Point, position)
			STATE_PROPERTY(SDL_Point, size)
		};

		const auto &properties = apply_properties(ButtonProperties(), parameters);

		return MouseArea
		{
			position = properties.position,
			size = properties.size,

			Rectangle
			{
				size = SDL_Point { properties.size.x, properties.size.y + 1 },
				position = properties.position,
				color = SDL_Color { 0x33, 0x33, 0x33, 0xFF }
			},

			Rectangle
			{
				size = properties.size,
				position = properties.position,
				color = SDL_Color { 0x99, 0x99, 0x99, 0xFF }
			},

			Rectangle
			{
				size = SDL_Point { properties.size.x - 2, properties.size.y - 2 },
				position = SDL_Point { properties.position.x + 1, properties.position.y + 1 },
				color = SDL_Color { 0xEF, 0xEF, 0xEF, 0xFF }
			}
		};
	}
};

struct MyApplication : public Application<MyApplication, State, DefaultStyle>
{
	static State increment_counter(const State &state)
	{
		return state.with_counter(state.counter + 1);
	}

	static State decrement_counter(const State &state)
	{
		return state.with_counter(state.counter - 1);
	}

	static auto layout(const State &state)
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
};

int main(int argc, char **argv)
{
	QApplication application(argc, argv);

	MyApplication app;
	app.run();

	return 0;
}
