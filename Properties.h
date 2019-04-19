#ifndef PROPERTIES_H
#define PROPERTIES_H

struct Property
{
};

#define API_PROPERTY(name)\
struct name##_Property\
{\
	template<typename TValue>\
	struct Setter : public Property\
	{\
		Setter(const TValue &value)\
			: value(value)\
		{\
		}\
\
		Setter() = default;\
		Setter(const Setter &other) = default;\
\
		template<typename TState>\
		TState apply(const TState &state) const\
		{\
			return state.with_##name (value);\
		}\
\
		TValue value;\
	};\
\
	template<typename TValue>\
	const auto operator =(TValue value) const\
	{\
		return Setter<TValue>(value);\
	}\
};\
\
name##_Property name;\

#define STATE_PROPERTY(type, name)\
auto with_##name (type name) const\
{\
	std::decay_t<decltype(*this)> copy(*this);\
	copy. name = name ;\
\
	return copy;\
}\
\
type name;\

API_PROPERTY(size)
API_PROPERTY(position)
API_PROPERTY(on_clicked)
API_PROPERTY(text)
API_PROPERTY(color)
API_PROPERTY(on_mouse_over)
API_PROPERTY(normal)

#endif // PROPERTIES_H
