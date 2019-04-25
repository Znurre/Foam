#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>

#include <SDL.h>
#include <GL/glew.h>

#include "Repack.h"

#include "incbin.h"

INCBIN(VertexShader, "shader.vert");
INCBIN(FragmentShader, "shader.frag");

template<typename TControl>
DrawCommand get_draw_command(const TControl &control)
{
	return control.draw_command;
}

template<typename TTuple, std::size_t ...TIndex>
auto extract_draw_commands(const TTuple &tuple, std::index_sequence<TIndex...>) -> std::array<DrawCommand, std::tuple_size_v<TTuple>>
{
	return { get_draw_command(std::get<std::tuple_size_v<TTuple> - (TIndex + 1)>(tuple))... };
}

template<typename TTuple>
auto extract_draw_commands(const TTuple &tuple)
{
	return extract_draw_commands(tuple, std::make_index_sequence<std::tuple_size_v<TTuple>>());
}

template<typename TApplication, typename TUserState, typename TStyle>
struct Application
{
	template<typename T>
	using DrawableControlTypePredicate = std::is_base_of<DrawableControl, T>;

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
	void run(TState state)
	{
		SDL_Event event;

		// This is unfortunate, but we apparently cannot rely on tail call optimization here :(
		while (true)
		{
			const auto &root = std::get<RootState>(state);

			SDL_WaitEvent(&event);

			if (event.type == SDL_QUIT)
			{
				return;
			}

			state = layout<Operation::Draw>(
				layout<Operation::Update>(
					repack(state, root.with_event(event))
				)
			);

			const auto &drawables = tuple_filter<DrawableControlTypePredicate>(state);
			const auto &commands = extract_draw_commands(drawables);

			glClear(GL_COLOR_BUFFER_BIT);

			GLuint buffer;

			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(commands), commands.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DrawCommand), (void *)sizeof(glm::mat3));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), (void *)0);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), (void *)(sizeof(glm::vec3) * 1));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), (void *)(sizeof(glm::vec3) * 2));

			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);

			glBindBuffer(GL_ARRAY_BUFFER, root.vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, root.ibo);

			glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0, commands.size());

			SDL_GL_SwapWindow(root.window);
		}
	}

	void run()
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		auto window = SDL_CreateWindow("Foam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
		auto context = SDL_GL_CreateContext(window);

		glewInit();

		SDL_GL_SetSwapInterval(0);

		RootState root;
		root.window = window;
		root.context = context;

		glGenVertexArrays(1, &root.vao);
		glBindVertexArray(root.vao);

		const auto program = glCreateProgram();

		const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const auto *vs_source = (const GLchar *)&gVertexShaderData;
		const auto *fs_source = (const GLchar *)&gFragmentShaderData;

		glShaderSource(vertexShader, 1, &vs_source, (GLint *)&gVertexShaderSize);
		glShaderSource(fragmentShader, 1, &fs_source, (GLint *)&gFragmentShaderSize);

		glCompileShader(vertexShader);
		glCompileShader(fragmentShader);

		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);

		glClearColor(0.f, 0.f, 0.f, 1.f);

		const auto &state = init_state();

		glGenBuffers(1, &root.vbo);
		glGenBuffers(1, &root.ibo);

		glUseProgram(program);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		static const GLfloat vertex_data[] =
		{
			0.f, 0.f,
			1.f, 0.f,
			1.f, 1.f,
			0.f, 1.f
		};

		glBindBuffer(GL_ARRAY_BUFFER, root.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

		static const GLuint index_data[] =
		{
			0, 1, 2, 3
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, root.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_STATIC_DRAW);

		const auto tuple = std::make_tuple(root, state);

		run(layout<Operation::Initialize>(tuple));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glUseProgram(0);
		glDeleteProgram(program);

		SDL_DestroyWindow(window);

		SDL_Quit();
	}
};

#endif // APPLICATION_H
