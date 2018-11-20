#include <SDL.h>

#include <tuple>

#include "Repack.h"

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

enum class VisualState
{
	Normal = 0,
	Highlight,
	Disabled
};

template<typename TUserState>
using Callback = TUserState (*)(const TUserState &state);

template<typename TState>
using UserState = typename std::tuple_element<std::tuple_size<TState>::value - 1, TState>::type;

template<typename TUserState>
struct ButtonStateBase : public BasicControlState
{
	Callback<TUserState> on_clicked;
};

template<int TId, typename TUserState>
struct ButtonState : public ButtonStateBase<TUserState>
{
	ButtonState<TId, TUserState> with_state(VisualState state) const
	{
		ButtonState<TId, TUserState> copy(*this);
		copy.state = state;

		return copy;
	}

	VisualState state;
};

template<typename TUserState>
struct CallbackPropertyValueApplier : public IPropertyValueApplier<ButtonStateBase<TUserState>, Callback<TUserState>>
{
	void apply(ButtonStateBase<TUserState> &state, const Callback<TUserState> &value) override
	{
		state.on_clicked = value;
	}
};

struct CallbackProperty
{
	template<typename TUserState>
	auto operator =(Callback<TUserState> callback)
	{
		static CallbackPropertyValueApplier<TUserState> applier;

		return PropertyValue<ButtonStateBase<TUserState>, Callback<TUserState>>(applier, callback);
	}
};

PositionPropertyValueApplier positionPropertyValueApplier;
SizePropertyValueApplier sizePropertyValueApplier;

Property<BasicControlState, SDL_Point> position(positionPropertyValueApplier);
Property<BasicControlState, SDL_Point> size(sizePropertyValueApplier);

CallbackProperty on_clicked;

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

template<Operation TOperation, typename TState, int TId, typename ...TProperties>
struct ButtonLogic
{
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Initialize, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...)
	{
		return tuple_prepend(ButtonState<TId, UserState<TState>>(), state);
	}
};

template<typename TState, int TId, typename ...TProperties>
struct ButtonLogic<Operation::Update, TState, TId, TProperties...>
{
	static auto invoke(const TState &state, TProperties &...properties)
	{
		// This should be const& in the future
		auto button = std::get<ButtonState<TId, UserState<TState>>>(state);

		apply_properties(button, properties...);

		return handle_events(
			calculate_state(
				repack(state, button)
			)
		);
	}

	static auto handle_events(const TState &state)
	{
		const auto &root = std::get<RootState>(state);
		const auto &user = std::get<UserState<TState>>(state);
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);

		if (button.on_clicked == nullptr)
		{
			return state;
		}

		if (root.event.button.type != SDL_MOUSEBUTTONDOWN)
		{
			return state;
		}

		const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
		const SDL_Point point = { root.event.motion.x, root.event.motion.y };

		if (SDL_PointInRect(&point, &rect))
		{
			return repack(state, button.on_clicked(user));
		}

		return state;
	}

	static auto calculate_state(const TState &state)
	{
		const auto &root = std::get<RootState>(state);
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);

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
		const auto &button = std::get<ButtonState<TId, UserState<TState>>>(state);
		const auto &root = std::get<RootState>(state);

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
