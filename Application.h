#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>
#include <vector>
#include <numeric>

#include <boost/range/join.hpp>

#include <SDL.h>

#include <GL/glew.h>

#include <freetype2/ft2build.h>

#include FT_FREETYPE_H

#include "Repack.h"

#include "incbin.h"

INCBIN(VertexShader, "shader.vert");
INCBIN(FragmentShader, "shader.frag");

#define offset(t, d) reinterpret_cast<void *>(offsetof(t, d))

glm::vec4 get_glyph_position(FT_Face face, uint x, uint y)
{
	return (x + face->glyph->bitmap.width) > TEXTURE_SIZE
		? glm::vec4(0, y + uint(face->height / 64), face->glyph->bitmap.width, face->glyph->bitmap.rows)
		: glm::vec4(x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows);
}

Glyph create_glyph(FT_Face face, size_t index, uint x, uint y)
{
	FT_Load_Char(face, index, FT_LOAD_RENDER);

	return
	{
		index,

		get_glyph_position(face, x, y),

		face->glyph->advance.x >> 6,
		face->glyph->advance.y >> 6,
		face->height / 64 - face->glyph->bitmap_top,
	};
}

template<typename TElement, std::size_t TSize, std::size_t ...TIndex>
std::array<TElement, TSize + 1> array_append(const TElement &element, const std::array<TElement, TSize> &array, std::index_sequence<TIndex...>)
{
	return { std::get<TIndex>(array)..., element };
}

template<typename TElement, std::size_t TSize>
std::array<TElement, TSize + 1> array_append(const TElement &element, const std::array<TElement, TSize> &array)
{
	return array_append(element, array, std::make_index_sequence<TSize>());
}

template<std::size_t TStart, std::size_t TEnd>
struct create_glyphs
{
	static auto value(FT_Face face, uint x, uint y)
	{
		const auto &glyph = create_glyph(face, TEnd - 1, x, y);
		const auto &trailing = create_glyphs<TStart, TEnd - 1>::value(face, uint(glyph.bounds.x + glyph.bounds.z), uint(glyph.bounds.y));

		return array_append(glyph, trailing);
	}
};

template<std::size_t TStart>
struct create_glyphs<TStart, TStart>
{
	static std::array<Glyph, 1> value(FT_Face face, uint x, uint y)
	{
		return { create_glyph(face, TStart - 1, x, y) };
	}
};

template<typename TControl>
auto get_draw_command(const TControl &control)
{
	return control.draw_commands;
}

template<typename TTuple, std::size_t ...TIndex>
immutable_vector<DrawCommand> extract_draw_commands(const TTuple &tuple, std::index_sequence<TIndex...>)
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

	virtual ~Application() = default;

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

			glBindBuffer(GL_ARRAY_BUFFER, root.commands);
			glBufferData(GL_ARRAY_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DrawCommand), offset(DrawCommand, color));
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), offset(DrawCommand, uv));
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), offset(DrawCommand, matrix));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), (void *)(sizeof(glm::vec3) * 1));
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(DrawCommand), (void *)(sizeof(glm::vec3) * 2));

			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);

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

		FT_Library library;
		FT_Face face;

		FT_Init_FreeType(&library);
		FT_New_Face(library, "/usr/share/fonts/cantarell/Cantarell-Regular.otf", 0, &face);
		FT_Set_Char_Size(face, 0, 11 * 64, 96, 96);

		RootState root;
		root.window = window;
		root.context = context;
		root.glyphs = create_glyphs<33, 128>::value(face, 0, 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glGenVertexArrays(1, &root.vao);
		glBindVertexArray(root.vao);

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &root.font);
		glBindTexture(GL_TEXTURE_2D, root.font);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

		for (const auto &glyph : root.glyphs)
		{
			FT_Load_Char(face, glyph.index, FT_LOAD_RENDER);

			glTexSubImage2D(GL_TEXTURE_2D, 0, int(glyph.bounds.x), int(glyph.bounds.y), int(glyph.bounds.z), int(glyph.bounds.w), GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		}

		const auto program = glCreateProgram();

		const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const auto *vs_source = reinterpret_cast<const GLchar *>(&gVertexShaderData);
		const auto *fs_source = reinterpret_cast<const GLchar *>(&gFragmentShaderData);

		glShaderSource(vertexShader, 1, &vs_source, reinterpret_cast<const GLint *>(&gVertexShaderSize));
		glShaderSource(fragmentShader, 1, &fs_source, reinterpret_cast<const GLint *>(&gFragmentShaderSize));

		glCompileShader(vertexShader);
		glCompileShader(fragmentShader);

		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);

		glClearColor(0.f, 0.f, 0.f, 1.f);

		const auto &state = init_state();

		glGenBuffers(1, &root.vbo);
		glGenBuffers(1, &root.ibo);
		glGenBuffers(1, &root.commands);

		glUseProgram(program);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);

		static const GLfloat vertex_data[] =
		{
			0.f, 0.f,
			1.f, 0.f,
			1.f, 1.f,
			0.f, 1.f
		};

		glBindBuffer(GL_ARRAY_BUFFER, root.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

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
		glDisableVertexAttribArray(5);

		glUseProgram(0);
		glDeleteProgram(program);

		SDL_DestroyWindow(window);

		SDL_Quit();
	}
};

#endif // APPLICATION_H
