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
			size = SDL_Point { control.size.x, control.size.y + 1 },
			position = control.position,
			color = SDL_Color { 0x33, 0x33, 0x33, 0xFF },

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = SDL_Color { 0x99, 0x99, 0x99, 0xFF }
			},

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 1 },
				color = SDL_Color { 0xEF, 0xEF, 0xEF, 0xFF }
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
			size = SDL_Point { control.size.x, control.size.y + 1 },
			position = control.position,
			color = SDL_Color { 0x33, 0x33, 0x33, 0xFF },

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = SDL_Color { 0xBB, 0xBB, 0xBB, 0xFF }
			},

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 1 },
				color = SDL_Color { 0xFF, 0xFF, 0xFF, 0xFF }
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
			size = SDL_Point { control.size.x, control.size.y + 1 },
			position = control.position,
			color = SDL_Color { 0x33, 0x33, 0x33, 0xFF },

			Rectangle
			{
				size = control.size,
				position = SDL_Point { control.position.x, control.position.y + 1 },
				color = SDL_Color { 0x99, 0x99, 0x99, 0xFF }
			},

			Rectangle
			{
				size = SDL_Point { control.size.x - 2, control.size.y - 2 },
				position = SDL_Point { control.position.x + 1, control.position.y + 2 },
				color = SDL_Color { 0xBB, 0xBB, 0xBB, 0xFF }
			}
		};
	}
};

using DefaultStyle = Style<ButtonStyle<DefaultButtonNormalTemplate, DefaultButtonHoverTemplate, DefaultButtonPressedTemplate>>;


#endif // DEFAULTSTYLE_H
