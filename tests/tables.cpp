#include <catch.hpp>
#include <luwra.hpp>
#include <string>

TEST_CASE("Path<Table, string>") {
	luwra::StateWrapper state;
	REQUIRE(state.runString("value = 1337") == LUA_OK);

	luwra::internal::Path<luwra::Table, std::string> path {state, "value"};

	state.push(path);
	REQUIRE(state.read<int>(-1) == 1337);
	REQUIRE(path.read<int>(state) == 1337);

	REQUIRE(lua_gettop(state) == 1);

	path.write(state, 13.37);

	state.push(path);
	REQUIRE(state.read<double>(-1) == 13.37);
	REQUIRE(path.read<double>(state) == 13.37);

	REQUIRE(lua_gettop(state) == 2);
}

TEST_CASE("Path<Reference, string>") {
	luwra::StateWrapper state;
	REQUIRE(state.runString("value = 1337") == LUA_OK);

	luwra::internal::Path<luwra::Reference, std::string> path {state.ref, "value"};

	state.push(path);
	REQUIRE(state.read<int>(-1) == 1337);
	REQUIRE(path.read<int>(state) == 1337);

	REQUIRE(lua_gettop(state) == 1);

	path.write(state, 13.37);

	state.push(path);
	REQUIRE(state.read<double>(-1) == 13.37);
	REQUIRE(path.read<double>(state) == 13.37);

	REQUIRE(lua_gettop(state) == 2);
}

TEST_CASE("Path<Path<Reference, string>, string>") {
	luwra::StateWrapper state;
	REQUIRE(state.runString("value = {field = 1337}") == LUA_OK);

	luwra::internal::Path<
		luwra::internal::Path<
			luwra::Reference,
			std::string
		>,
		std::string
	> path {{state.ref, "value"}, "field"};

	state.push(path);
	REQUIRE(state.read<int>(-1) == 1337);
	REQUIRE(path.read<int>(state) == 1337);

	REQUIRE(lua_gettop(state) == 1);

	path.write(state, 13.37);

	state.push(path);
	REQUIRE(state.read<double>(-1) == 13.37);
	REQUIRE(path.read<double>(state) == 13.37);

	REQUIRE(lua_gettop(state) == 2);

	REQUIRE(state.runString("return value.field") == LUA_OK);
	REQUIRE(state.read<double>(-1) == 13.37);
}
