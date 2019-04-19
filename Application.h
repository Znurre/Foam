#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL.h>

#include "Repack.h"

template<typename TApplication, typename TUserState, typename TStyle>
struct Application
{
	virtual TUserState init_state()
	{
		return TUserState();
	}

	virtual TUserState update_state(const TUserState &state)
	{
		return state;
	}

	template <Operation TOperation, typename TState>
	auto layout(const TState &state)
	{
		const auto &user = std::get<TUserState>(state);
		const auto &root = TApplication::layout(user);
		const auto &context = make_context<TOperation, TStyle>(state);
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

	void run()
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		auto window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
		auto surface = SDL_GetWindowSurface(window);

		const auto &state = init_state();

		RootState root;
		root.window = window;
		root.surface = surface;

		const auto tuple = std::make_tuple(root, state);

		run(layout<Operation::Initialize>(tuple));
	}
};

#endif // APPLICATION_H
