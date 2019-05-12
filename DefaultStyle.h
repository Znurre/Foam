#ifndef DEFAULTSTYLE_H
#define DEFAULTSTYLE_H

#include "Style.h"
#include "Rectangle.h"
#include "Text.h"

struct DefaultButtonNormalTemplate
{
	template<typename TControl>
	static auto layout(const TControl &control)
	{
		return Rectangle
		{
			size = glm::vec2(control.size.x, control.size.y + 2),
			position = control.position,
			color = 0xffb98029,

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = 0xfff3991d
			},

			Text
			{
				size = control.size,
				position = control.position,
				text = control.text,
				color = 0xffffffff
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
			size = glm::vec2(control.size.x, control.size.y + 2),
			position = control.position,
			color = 0xffb98029,

			Rectangle
			{
				size = control.size,
				position = control.position,
				color = 0xffe9ae3d
			},

			Text
			{
				size = control.size,
				position = control.position,
				text = control.text,
				color = 0xffffffff
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
			size = glm::vec2(control.size.x, control.size.y + 2),
			position = control.position,
			color = 0xffb98029,

			Text
			{
				size = control.size,
				position = control.position,
				text = control.text,
				color = 0xffffffff
			}
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
			color = 0xffc7c3bd,

			Rectangle
			{
				size = glm::vec2(control.size.x - 2, control.size.y - 2),
				position = glm::vec2(control.position.x + 1, control.position.y + 1),
				color = 0xfffcfcfc
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
			color = 0xffa6a595,

			Rectangle
			{
				size = glm::vec2(control.size.x - 2, control.size.y - 2),
				position = glm::vec2(control.position.x + 1, control.position.y + 1),
				color = 0xfffcfcfc
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
			color = 0xff8d8c7f,

			Rectangle
			{
				size = glm::vec2(control.size.x - 2, control.size.y - 2),
				position = glm::vec2(control.position.x + 1, control.position.y + 1),
				color = 0xfffcfcfc
			}
		};
	}
};

using DefaultStyle = Style<
	ButtonStyle<DefaultButtonNormalTemplate, DefaultButtonHoverTemplate, DefaultButtonPressedTemplate>,
	TextBoxStyle<DefaultTextBoxNormalTemplate, DefaultTextBoxHoverTemplate, DefaultTextBoxFocusedTemplate>
	>;


#endif // DEFAULTSTYLE_H
