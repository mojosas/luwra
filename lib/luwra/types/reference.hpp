/* Luwra
 * Minimal-overhead Lua wrapper for C++
 *
 * Copyright (C) 2016, Ole Krüger <ole@vprsm.de>
 */

#ifndef LUWRA_TYPES_REFERENCE_H_
#define LUWRA_TYPES_REFERENCE_H_

#include "../common.hpp"
#include "../values.hpp"

#include <memory>

LUWRA_NS_BEGIN

namespace internal {
	// Create reference the value pointed to by `index`. Does not remove the referenced value.
	inline
	int referenceValue(State* state, int index) {
		lua_pushvalue(state, index);
		return luaL_ref(state, LUA_REGISTRYINDEX);
	}

	// Implementation of a reference which takes care of the lifetime of a Lua reference
	struct ReferenceImpl {
		State* state;
		int ref;
		bool autoUnref = true;

		// Reference a value at an index.
		inline
		ReferenceImpl(State* state, int indexOrRef, bool isIndex = true):
			state(state),
			ref(isIndex ? referenceValue(state, indexOrRef) : indexOrRef),
			autoUnref(isIndex)
		{}

		// Reference the value on top of stack.
		inline
		ReferenceImpl(State* state):
			state(state),
			ref(luaL_ref(state, LUA_REGISTRYINDEX))
		{}

		// A (smart) pointer to an instance may be copied and moved, but the instance itself must
		// not be copied or moved. This allows us to have only one instance of `ReferenceImpl` per
		// Lua reference.
		ReferenceImpl(const ReferenceImpl& other) = delete;
		ReferenceImpl(ReferenceImpl&& other) = delete;
		ReferenceImpl& operator =(const ReferenceImpl&) = delete;
		ReferenceImpl& operator =(ReferenceImpl&&) = delete;

		inline
		~ReferenceImpl() {
			if (ref >= 0 && autoUnref) luaL_unref(state, LUA_REGISTRYINDEX, ref);
		}

		// Push the referenced value onto the stack.
		inline
		void push() const {
			lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
		}

		// Reset the referenced value.
		inline
		void update() const {
			lua_rawseti(state, LUA_REGISTRYINDEX, ref);
		}
	};

	using SharedReferenceImpl = std::shared_ptr<const internal::ReferenceImpl>;
}

/// %Reference cell which contains a Lua value
struct Reference {
	const internal::SharedReferenceImpl impl;

	/// Copy the value at the given index or reference into a new reference cell.
	/// The value will not be removed.
	///
	/// \param state      Lua state
	/// \param indexOrRef Index or reference identifier
	/// \param isIndex    Is `indexOrRef` an index?
	inline
	Reference(State* state, int indexOrRef = -1, bool isIndex = true):
		impl(std::make_shared<internal::ReferenceImpl>(state, indexOrRef, isIndex))
	{}
};

/// Enables referencing/dereferencing values
template <>
struct Value<Reference> {
	static inline
	Reference read(State* state, int index) {
		return {state, index, true};
	}

	static inline
	void push(State* state, const Reference& value) {
		value.impl->push();

		if (value.impl->state != state)
			lua_xmove(value.impl->state, state, 1);
	}
};

LUWRA_NS_END

#endif
