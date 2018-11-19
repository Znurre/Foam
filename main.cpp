#include <SDL.h>

#include <tuple>

template<typename TTuple, typename TAddition>
auto extend_tuple(const TTuple &tuple, const TAddition &addition)
{
	return std::tuple_cat(tuple, std::make_tuple(addition));
}

template<int TIndex, typename TTuple>
using Element = typename std::tuple_element<TIndex, TTuple>::type;

template<int TIndex, typename TTuple, typename TElement>
struct Repack
{
	auto operator()(const TTuple &tuple, const TElement &element)
	{
		return extend_tuple(Repack<TIndex - 1, TTuple, TElement>()(tuple, element), std::get<TIndex>(tuple));
	}
};

template<int TIndex, typename TTuple>
struct Repack<TIndex, TTuple, Element<TIndex, TTuple>>
{
	auto operator()(const TTuple &tuple, const Element<TIndex, TTuple> &element)
	{
		return extend_tuple(Repack<TIndex - 1, TTuple, Element<TIndex, TTuple>>()(tuple, element), element);
	}
};

template<typename TTuple, typename TElement>
struct Repack<0, TTuple, TElement>
{
	auto operator()(const TTuple &tuple, const TElement &)
	{
		return std::make_tuple(std::get<0>(tuple));
	}
};

template<typename TTuple>
struct Repack<0, TTuple, Element<0, TTuple>>
{
	auto operator()(const TTuple &, const Element<0, TTuple> &element)
	{
		return std::make_tuple(element);
	}
};

template<typename TTuple, typename TElement>
TTuple repack(const TTuple &tuple, const TElement &element)
{
	return Repack<std::tuple_size<TTuple>::value - 1, TTuple, TElement>()(tuple, element);
}

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
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Event *event;
};

template<typename TState>
class Logic
{
	public:
		virtual TState invoke(const TState &state) = 0;
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
class ButtonLogic
{
};

template<typename TState, int TId, typename ...TProperties>
class ButtonLogic<Operation::Initialize, TState, TId, TProperties...>
{
	public:
		auto invoke(const TState &state, TProperties &...)
		{
			return std::tuple_cat(std::make_tuple(ButtonState<TId>()), state);
		}
};

template<typename TState, int TId, typename ...TProperties>
class ButtonLogic<Operation::Update, TState, TId, TProperties...>
{
	public:
		auto invoke(const TState &state, TProperties &...properties)
		{
			const RootState &root = std::get<RootState>(state);

			ButtonState<TId> button = std::get<ButtonState<TId>>(state);

			apply_properties(button, properties...);

			const SDL_Rect rect = { button.position.x, button.position.y, button.size.x, button.size.y };
			const SDL_Point point = { root.event->motion.x, root.event->motion.y };

			if (SDL_PointInRect(&point, &rect))
			{
				return repack(state, button.with_state(VisualState::Highlight));
			}

			return repack(state, button.with_state(VisualState::Normal));
		}
};

template<typename TState, int TId, typename ...TProperties>
class ButtonLogic<Operation::Draw, TState, TId, TProperties...>
{
	public:
		auto invoke(const TState &state, TProperties &...)
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
auto Button(const TState &state, TProperties &...properties)
{
	ButtonLogic<TOperation, TState, TId, TProperties...> logic;

	return logic.invoke(state, properties...);
}

template<typename TState, typename ...TChild>
TState Horizontal(TState &state, const TChild &...)
{
	return state;
}

template<typename TState, typename ...TChild>
TState Vertical(TState state, const TChild &...)
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
	const RootState &root = std::get<RootState>(state);

	SDL_UpdateWindowSurface(root.window);

	SDL_FillRect(root.surface, nullptr, 0x00000000);
	SDL_WaitEvent(root.event);

	return run(layout<Operation::Draw>(layout<Operation::Update>(state)));
}

int main(int, char **)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *window = SDL_CreateWindow("Pure", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
	SDL_Surface *surface = SDL_GetWindowSurface(window);

	State state;

	RootState ws;
	ws.window = window;
	ws.surface = surface;
	ws.event = new SDL_Event();

	auto tuple = std::make_tuple(ws, state);
	auto init = layout<Operation::Initialize>(tuple);

	run(init);

	return 0;
}
