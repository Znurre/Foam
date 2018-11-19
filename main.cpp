#include <SDL2/SDL.h>

#include <tuple>

#include "Repack.h"

struct State
{
};

template<typename TState, typename TValue>
class IPropertyValueApplier
{
	public:
		virtual void apply(TState &state, const TValue &value) = 0;
};

struct BasicControlState
{
	SDL_Point position;
	SDL_Point size;
};

class PositionPropertyValueApplier : public IPropertyValueApplier<BasicControlState, SDL_Point>
{
	public:
		void apply(BasicControlState &state, const SDL_Point &value) override
		{
			state.position = value;
		}
};

class SizePropertyValueApplier : public IPropertyValueApplier<BasicControlState, SDL_Point>
{
	public:
		void apply(BasicControlState &state, const SDL_Point &value) override
		{
			state.size = value;
		}
};

template<typename TState, typename TValue>
class PropertyValue
{
	public:
		PropertyValue(IPropertyValueApplier<TState, TValue> &applier, const TValue &value)
			: m_applier(applier)
			, m_value(value)
		{
		}

		void apply(TState &state)
		{
			m_applier.apply(state, m_value);
		}

	private:
		IPropertyValueApplier<TState, TValue> &m_applier;
		TValue m_value;
};

template<typename TState, typename TValue>
class Property
{
	public:
		Property(IPropertyValueApplier<TState, TValue> &applier)
			: m_applier(applier)
		{
		}

		PropertyValue<TState, TValue> operator =(TValue value)
		{
			return PropertyValue<TState, TValue>(m_applier, value);
		}

	private:
		IPropertyValueApplier<TState, TValue> &m_applier;
};

PositionPropertyValueApplier positionPropertyValueApplier;
SizePropertyValueApplier sizePropertyValueApplier;

Property<BasicControlState, SDL_Point> position(positionPropertyValueApplier);
Property<BasicControlState, SDL_Point> size(sizePropertyValueApplier);

enum class VisualState
{
	Normal = 0,
	Highlight,
	Disabled
};

enum class Operation
{
	Initialize,
	Update,
	Draw
};

template<typename TState, typename TProperty>
void apply_properties(TState &state, TProperty &property)
{
	property.apply(state);
}

template<typename TState, typename TProperty, typename ...TProperties>
void apply_properties(TState &state, TProperty &property, TProperties&... properties)
{
	property.apply(state);

	apply_properties(state, properties...);
}

struct RootState
{
	RootState with_event(const SDL_Event &event) const
	{
		RootState copy(*this);
		copy.event = event;

		return copy;
	}

	SDL_Window *window;
	SDL_Surface *surface;

	SDL_Event event;
};

template<int TId>
struct ButtonState : public BasicControlState
{
	ButtonState<TId> with_state(VisualState state) const
	{
		ButtonState<TId> copy(*this);
		copy.state = state;

		return copy;
	}

	VisualState state;
};

template<Operation TOperation, typename TState, int TId, typename ...TProperties>
struct ButtonLogic
{
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Initialize, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		return std::tuple_cat(std::make_tuple(ButtonState<TId>()), state);
	}
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Update, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...properties)
	{
		const RootState &root = std::get<RootState>(state);

		ButtonState<TId> button = std::get<ButtonState<TId>>(state);

		apply_properties(button, properties...);

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (SDL_PointInRect(&point, &rect))
		{
			return repack(state, button.with_state(VisualState::Highlight));
		}

		return repack(state, button.with_state(VisualState::Normal));
	}
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Draw, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		const ButtonState<TId> &button = std::get<ButtonState<TId>>(state);
		const RootState &root = std::get<RootState>(state);

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };

		if (button.state == VisualState::Highlight)
		{
			SDL_FillRect(root.surface, &rect, 0xFFFF0000);
		}
		else
		{
			SDL_FillRect(root.surface, &rect, 0xFFFFFF00);
		}

		return state;
	}
};

template<int TId, Operation TOperation, typename TState, typename ...TProperties>
auto Button(const TState &state, TProperties ...properties)
{
	return ButtonLogic<TOperation, TState, TId, TProperties...>::invoke(state, properties...);
}

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

template<Operation TOperation, typename TState>
auto layout(const TState &state)
{
	return
		Button<1, TOperation>(
			Button<2, TOperation>(state
				, position = SDL_Point { 120, 10 }
				, size = SDL_Point { 100, 30 }
			)
			, position = SDL_Point { 10, 10 }
			, size = SDL_Point { 100, 30 }
		);
}

template<typename TState>
auto run(const TState &state) -> decltype(layout<Operation::Update>(state))
{
	SDL_Event event;

	const RootState &root = std::get<RootState>(state);

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

	SDL_Window *window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
	SDL_Surface *surface = SDL_GetWindowSurface(window);

	State state;

	RootState root;
	root.window = window;
	root.surface = surface;

	const auto tuple = std::make_tuple(root, state);

	run(layout<Operation::Initialize>(tuple));

	return 0;
}
