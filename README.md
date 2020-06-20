A small library for creating polymorphic handles with inline storage.


```c++
#include <poly_handle/poly.hpp>
#include <iostream>

////////////////////////////
// 1. Define your concept //
////////////////////////////
struct printable_concept : poly::concept
{
	std::ostream& (*_print)(void const*, std::ostream&);
};

///////////////////////////////
// 2. implement your concept //
///////////////////////////////
template<typename Base>
struct printable_model : Base
{
	using Base::Base;
	static std::ostream& _print(void const* self, std::ostream& os)
	{
		return os << Base::data(self);
	}
	static constexpr printable_concept vtable{ Base::vtable, _print };
};

//////////////////////////////
// 3. implement your handle //
//////////////////////////////
using printable_base = poly::handle<printable_concept, printable_model>;
struct printable : printable_base
{
	using printable_base::printable_base;
	friend std::ostream& operator<<(std::ostream& os, printable const& p)
	{
		return p.poly_call(&concept::_print, os);
	}
};

///////////////
// 4. Use it //
///////////////
int main() {
	printable x = 15;
	std::cout << x << '\n';
}
```

