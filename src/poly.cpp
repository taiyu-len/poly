#include <poly_handle/poly.hpp>
#include <doctest/doctest.h>
namespace {
struct test_concept : poly::concept
{
	void (*_call)(void*, int x);
};

template<typename Base>
struct test_model : Base
{
	using Base::Base, Base::data;
	static void _call(void* self, int x)
	{
		data(self)(x);
	}
	static constexpr test_concept vtable{Base::vtable, _call};
};

using handle_base = poly::handle<test_concept, test_model>;
struct handler : handle_base
{
	using handle_base::handle_base;
	void operator()(int x)
	{
		poly_call(&concept::_call, x);
	}
};


template<typename T>
struct test_obj : T
{
	int& mov;
	int& des;

	test_obj(int& x, int& y)
		:mov(x), des(y)
	{}
	test_obj(test_obj&& x):
		mov(++x.mov), des(x.des)
	{}
	~test_obj()
	{
		++des;
	}

	void operator()(int) {}
};

struct small_data{};
struct big_data { double x[16]; };

using small_test_obj = test_obj<small_data>;
using big_test_obj  = test_obj<big_data>;

TEST_CASE("poly::test") {
	handler x = [](int y) { REQUIRE(y == 5); };
	x(5);

	x = [d=big_data{}](int y){ REQUIRE(y == 5); };
	x(5);

	int moved = 0;
	int destroyed = 0;
	{
		handler y = small_test_obj{moved, destroyed};
		// temporary object moved into internal storage,
		// temporary object destroyed
		REQUIRE(moved == 1);
		REQUIRE(destroyed == 1);

		// object in y moved into z,
		// object in y not destryoed, just moved from
		handler z = std::move(y);
		REQUIRE(moved == 2);
		REQUIRE(destroyed == 1);
	}
	// object in y and z both destroyed.
	REQUIRE(destroyed == 3);

	{
		moved = 0;
		destroyed = 0;
		handler y = big_test_obj{moved, destroyed};
		// temporary object moved into unique_ptr
		// temporary object destroyed
		REQUIRE(moved == 1);
		REQUIRE(destroyed == 1);

		// object in y not moved into z, unique_ptr moved
		// object in y not destroyed, still in unique_ptr
		handler z = std::move(y);
		REQUIRE(moved == 1);
		REQUIRE(destroyed == 1);
	}
	// object in unique_ptr destroyed
	REQUIRE(destroyed == 2);
}
} // namespace
