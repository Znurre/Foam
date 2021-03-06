#include <tuple>
#include <sstream>

#include "Application.h"
#include "Properties.h"
#include "Button.h"
#include "Rectangle.h"
#include "MouseArea.h"
#include "Component.h"
#include "DefaultStyle.h"
#include "TextBox.h"

struct State
{
	State()
		: counter(0)
		, frames(0)
		, fps(0)
		, time({0, 0})
	{
	}

	State with_counter(int counter) const
	{
		State copy(*this);
		copy.counter = counter;

		return copy;
	}

	State increment_fps() const
	{
		State copy(*this);
		copy.frames++;

		return copy;
	}

	State swap_fps() const
	{
		State copy(*this);
		copy.fps = frames;
		copy.frames = 0;

		return copy;
	}

	const std::string get_fps() const
	{
		return std::to_string(fps);
	}

	const std::string get_button_text() const
	{
		std::ostringstream stream;
		stream << "Value: " << counter;

		return stream.str();
	}

	int counter;
	int frames;
	int fps;

	timespec time;
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
	State init_state() override
	{
		State state;

		clock_gettime(CLOCK_MONOTONIC, &state.time);

		return state;
	}

	State update_state(const State &state) override
	{
		State copy(state);

		clock_gettime(CLOCK_MONOTONIC, &copy.time);

		if ((copy.time.tv_sec - state.time.tv_sec) >= 1)
		{
			return copy.swap_fps();
		}

		return copy.increment_fps();
	}

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
			TextBox
			{
				position = glm::vec2(30, 30),
				size = glm::vec2(210, 30)
			},

			TextBox
			{
				position = glm::vec2(30, 70),
				size = glm::vec2(210, 30)
			},

			Button
			{
				position = glm::vec2(30, 110 + state.counter * 5),
				size = glm::vec2(100, 30),
				on_clicked = &increment_counter,
				text = state.get_button_text()
			},

			Button
			{
				position = glm::vec2(140, 110 - state.counter * 5),
				size = glm::vec2(100, 30),
				on_clicked = &decrement_counter,
				text = state.get_button_text()
			},

			position = glm::vec2(10, 10),
			size = glm::vec2(250, 150),
			color = 0xfffcfcfc,

			Text
			{
				position = glm::vec2(10, 170),
				size = glm::vec2(250, 20),
				color = 0xffffffff,
				text = state.get_fps()
			}
		};
	}
};

int main()
{
	MyApplication app;
	app.run();

	return 0;
}
