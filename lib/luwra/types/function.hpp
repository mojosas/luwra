/* Luwra
 * Minimal-overhead Lua wrapper for C++
 *
 * Copyright (C) 2015, Ole Krüger <ole@vprsm.de>
 */

#ifndef LUWRA_TYPES_FUNCTION_H_
#define LUWRA_TYPES_FUNCTION_H_

#include "../common.hpp"
#include "../values.hpp"
#include "../stack.hpp"
#include "reference.hpp"

#include <utility>
#include <functional>

LUWRA_NS_BEGIN

/// A callable Lua value.
///
/// \tparam Ret Expected return type
template <typename Ret>
struct Function {
	/// Internal reference to the Lua value
	Reference ref;

	/// Create from reference.
	inline
	Function(const Reference& ref):
		ref(ref)
	{}

	/// Create from callable on the stack.
	inline
	Function(State* state, int index):
		ref(state, index)
	{
		int type = lua_type(state, index);
		if (type != LUA_TTABLE && type != LUA_TUSERDATA && type != LUA_TFUNCTION)
			luaL_argerror(state, index, "Expected table, userdata or function");
	}

	/// Convert from an existing @ref Function.
	template <typename OtherRet> inline
	Function(const Function<OtherRet>& other):
		ref(other.ref)
	{}

	/// Invoke the callable without arguments.
	inline
	Ret operator ()() const {
		ref.impl->push();

		lua_call(ref.impl->state, 0, 1);
		Ret returnValue = read<Ret>(ref.impl->state, -1);

		lua_pop(ref.impl->state, 1);
		return returnValue;
	}

	/// Invoke the callable with arguments.
	template <typename... Args> inline
	Ret operator ()(Args&&... args) const {
		ref.impl->push();
		push(ref.impl->state, std::forward<Args>(args)...);

		lua_call(ref.impl->state, sizeof...(Args), 1);
		Ret returnValue = read<Ret>(ref.impl->state, -1);

		lua_pop(ref.impl->state, 1);
		return returnValue;
	}
};

/// A callable Lua value without a return value.
template <>
struct Function<void> {
	/// Internal reference to the Lua value
	Reference ref;

	/// Create from reference.
	inline
	Function(const Reference& ref):
		ref(ref)
	{}

	/// Create from callable on the stack.
	inline
	Function(State* state, int index):
		ref(state, index)
	{
		int type = lua_type(state, index);
		if (type != LUA_TTABLE && type != LUA_TUSERDATA && type != LUA_TFUNCTION)
			luaL_argerror(state, index, "Expected table, userdata or function");
	}

	/// Convert from an existing @ref Function.
	template <typename OtherRet> inline
	Function(const Function<OtherRet>& other):
		ref(other.ref)
	{}

	/// Invoke the callable without arguments.
	inline
	void operator ()() const {
		ref.impl->push();
		lua_call(ref.impl->state, 0, 0);
	}

	/// Invoke the callable with arguments.
	template <typename... Args> inline
	void operator ()(Args&&... args) const {
		ref.impl->push();
		push(ref.impl->state, std::forward<Args>(args)...);

		lua_call(ref.impl->state, sizeof...(Args), 0);
	}
};

/// Enables reading/pushing Lua functions
template <typename Ret>
struct Value<Function<Ret>> {
	static inline
	Function<Ret> read(State* state, int index) {
		return {state, index};
	}

	static inline
	void push(State* state, const Function<Ret>& func) {
		luwra::push(state, func.ref);
	}
};

/// Enables reading Lua functions as `std::function`
template <typename Ret, typename... Args>
struct Value<std::function<Ret (Args...)>> {
	static inline
	std::function<Ret (Args...)> read(State* state, int index) {
		return {Value<Function<Ret>>::read(state, index)};
	}
};

/// Enables pushing for C functions
template <>
struct Value<CFunction> {
	static inline
	void push(State* state, CFunction fun) {
		lua_pushcfunction(state, fun);
	}
};

LUWRA_NS_END

#endif
