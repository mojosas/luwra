/* Luwra
 * Minimal-overhead Lua wrapper for C++
 *
 * Copyright (C) 2015, Ole Krüger <ole@vprsm.de>
 */

#ifndef LUWRA_AUXILIARY_H_
#define LUWRA_AUXILIARY_H_

#include "common.hpp"
#include "types.hpp"

#include <vector>
#include <utility>

LUWRA_NS_BEGIN

/**
 * Check if two values are equal.
 */
static inline
bool equal(State* state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
	return lua_equal(state, index1, index2);
#else
	return lua_compare(state, index1, index2, LUA_OPEQ);
#endif
}

/**
 * Register a value as a global.
 */
template <typename V> static inline
void setGlobal(State* state, const std::string& name, V value) {
	assert(1 == push(state, value));
	lua_setglobal(state, name.c_str());
}

/**
 * Retrieve a global value.
 */
template <typename V> static inline
V getGlobal(State* state, const std::string& name) {
	lua_getglobal(state, name.c_str());

	V instance = read<V>(state, -1);
	lua_pop(state, 1);

	return instance;
}

namespace internal {
	template <typename K, typename V, typename... R>
	struct EntryPusher {
		static inline
		void push(State* state, int index, K&& key, V&& value, R&&... rest) {
			EntryPusher<K, V>::push(state, index, std::forward<K>(key), std::forward<V>(value));
			EntryPusher<R...>::push(state, index, std::forward<R>(rest)...);
		}
	};

	template <typename K, typename V>
	struct EntryPusher<K, V> {
		static inline
		void push(State* state, int index, K&& key, V&& value) {
			assert(1 == luwra::push(state, key));
			assert(1 == luwra::push(state, value));
			lua_rawset(state, index < 0 ? index - 2 : index);
		}
	};
}

/**
 * Set multiple fields at once. Allows you to provide multiple key-value pairs.
 */
template <typename... R> static inline
void setFields(State* state, int index, R&&... args) {
	static_assert(sizeof...(R) % 2 == 0, "Field parameters must appear in pairs");
	internal::EntryPusher<R...>::push(state, index, std::forward<R>(args)...);
}

/**
 *
 */
using FieldVector = std::vector<std::pair<Pushable, Pushable>>;

/**
 *
 */
static inline
void setFields(State* state, int index, const FieldVector& fields) {
	if (index < 0)
		index = lua_gettop(state) + (index + 1);

	for (const auto& pair: fields) {
		pair.first.push(state);
		pair.second.push(state);
		lua_rawset(state, index);
	}
}

template <>
struct Value<FieldVector> {
	static inline
	size_t push(State* state, const FieldVector& fields) {
		lua_newtable(state);
		setFields(state, -1, fields);
		return 1;
	}
};

/**
 * Retrieve a field from a table.
 */
template <typename V, typename K> static inline
V getField(State* state, int index, K key) {
	if (index < 0)
		index = lua_gettop(state) + (index + 1);

	assert(push<K>(state, key) == 1);
	lua_rawget(state, index);

	V value = read<V>(state, -1);
	lua_pop(state, 1);

	return value;
}

LUWRA_NS_END

#endif
