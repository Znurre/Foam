#ifndef DEFAULTSTYLE_H
#define DEFAULTSTYLE_H

#include "Style.h"
#include "Rectangle.h"

struct DefaultButtonNormalTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = SDL_Point { control.size.x, control.size.y + 2 },
			position = control.position,
			color = SDL_Color { 0xb9, 0x80, 0x29, 0xFF },

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = SDL_Color { 0xf3, 0x99, 0x1d, 0xFF }
			}
		};
	}
};

struct DefaultButtonHoverTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = SDL_Point { control.size.x, control.size.y + 2 },
			position = control.position,
			color = SDL_Color { 0xb9, 0x80, 0x29, 0xFF },

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = SDL_Color { 0xe9, 0xae, 0x3d, 0xFF }
			}
		};
	}
};

struct DefaultButtonPressedTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = SDL_Point { control.size.x, control.size.y + 2 },
			position = control.position,
			color = SDL_Color { 0xb9, 0x80, 0x29, 0xFF }
		};
	}
};

struct DefaultTextBoxNormalTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = control.size,
			position = control.position,
			color = SDL_Color { 0xc7, 0xc3, 0xbd, 0xFF },

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 1 },
				color = SDL_Color { 0xfc, 0xfc, 0xfc, 0xff }
			}
		};
	}
};

struct DefaultTextBoxHoverTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = control.size,
			position = control.position,
			color = SDL_Color { 0xa6, 0xa5, 0x95, 0xFF },

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 1 },
				color = SDL_Color { 0xfc, 0xfc, 0xfc, 0xff }
			}
		};
	}
};

struct DefaultTextBoxFocusedTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = control.size,
			position = control.position,
			color = SDL_Color { 0x8d, 0x8c, 0x7f, 0xFF },

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 1 },
				color = SDL_Color { 0xfc, 0xfc, 0xfc, 0xff }
			}
		};
	}
};

using DefaultStyle = Style<
	ButtonStyle<DefaultButtonNormalTemplate, DefaultButtonHoverTemplate, DefaultButtonPressedTemplate>,
	TextBoxStyle<DefaultTextBoxNormalTemplate, DefaultTextBoxHoverTemplate, DefaultTextBoxFocusedTemplate>
	>;


#endif // DEFAULTSTYLE_H
