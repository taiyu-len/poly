#include <memory>
// adapted from http://stlab.cc/tips/small-object-optimizations.html
//
// Creates a type erased wrapper around a concept.
/* usage.
 *
 * 1. define a concept.
 * struct my_concept : poly::concept
 * {
 *    void (*_func)(void*, arguments)
 * };
 *
 * The concept contains a list of functions you want your handle to contain.
 *
 * 2. define a model
 * template<typename Base>
 * struct my_model : Base
 * {
 *   using Base::Base;
 *   static void _func(void* self, arguments)
 *   {
 *     auto& x = Base::data(self); // get back the type erased object
 *     x.func(arguments); // do stuff with your object
 *   }
 *   static constexpr my_concept vtable{Base::vtable, _func};
 * };
 *
 * The model contains the actual implementation of the functions contained in
 * the concept.
 *
 * The model must take a template paramter and use it as the base class.
 * the type passed in is the poly::model<T, is_small> type;
 *
 * the Base::Base will provide the constructors.
 * the Base::data(self) will provide a reference to the underlying type.
 * the concept vtable must be a constexpr object and defined with the
 * appropriate functions.
 *
 * Base::underlying_type contains the underlying type.
 *
 * 3. The handle
 * using my_base = poly::handle<my_concept, my_model, 8>;
 * struct my_handle : my_base
 * {
 *   using my_base::my_base;
 *   void func(arguments) {
 *     poly_call(&concept::_func, arguments);
 *   }
 * }
 *
 * Your handle provides the real api for using the wrapped objects.
 * the poly::handle provides the appropriates construtors.
 * poly::handle::concept is a typedef for your concept,
 * poly::handle::call takes a pointer to member from the concept, and the
 * arguments to be passed into it.
 *
 *
 * finally you can use your handle.
 * its as easy as
 *   handle x = some_type{};
 *   x.func();
 */

namespace poly {

struct concept
{
	void (*_dtor)(void*) noexcept = nullptr;
	void (*_move)(void*, void*) noexcept = nullptr;
};

template<typename T, bool is_small>
struct model;

template<typename T>
struct model<T, true>
{
	template<typename G>
	model(G&& x)
	: _data{std::forward<G>(x)}
	{
		// NOOP
	}

	using underlying_type = T;

	static auto data(void* self) noexcept -> T&
	{
		return static_cast<model*>(self)->_data;
	}

	static auto data(void const* self) noexcept -> T const&
	{
		return static_cast<model const*>(self)->_data;
	}
private:
	static void _dtor(void* self) noexcept
	{
		static_cast<model*>(self)->~model();
	}
	static void _move(void* self, void* x) noexcept
	{
		new (x) model(std::move(*static_cast<model*>(self)));
	}
protected:

	T _data;
	static constexpr concept vtable{_dtor, _move};
};

template<typename T>
struct model<T, false> : model<std::unique_ptr<T>, true>
{
	template<typename G>
	model(G&& x)
	: model<std::unique_ptr<T>, true>(std::make_unique<T>(std::forward<G>(x)))
	{
		// NOOP
	}

	using underlying_type = T;

	static auto data(void* self) noexcept -> T&
	{
		return *static_cast<model*>(self)->_data;
	}

	static auto data(void const* self) noexcept -> T const&
	{
		return *static_cast<model const*>(self)->_data;
	}
};

static constexpr size_t default_size = sizeof(void*) * 4;

template<
	typename Concept,
	template<typename> class Model,
	size_t small_size = default_size
>
struct handle
{
public:
	static_assert(small_size >= sizeof(std::unique_ptr<void>), "small_size too small");

	template<typename T>
	handle(T && x)
	{
		using small_model_t = Model<poly::model<std::decay_t<T>, true>>;
		constexpr bool is_small = sizeof(small_model_t) <= small_size;
		using model_t = Model<poly::model<std::decay_t<T>, is_small>>;
		new (&_model) model_t(std::forward<T>(x));
		_concept = &model_t::vtable;
	}
	~handle() noexcept
	{
		_concept->_dtor(&_model);
	}
	handle(handle&& x) noexcept: _concept(x._concept)
	{
		_concept->_move(&x._model, &_model);
	}
	handle& operator=(handle&& x) noexcept
	{
		if (this != &x)
		{
			_concept->_dtor(&_model);
			_concept = x._concept;
			_concept->_move(&x._model, &_model);
		}
		return *this;
	}

protected:
	template<typename F, typename... Args>
	auto poly_call(F f, Args&&... args) -> decltype(auto) {
		return (_concept->*f)(&_model, std::forward<Args>(args)...);
	}

	template<typename F, typename... Args>
	auto poly_call(F f, Args&&... args) const -> decltype(auto) {
		return (_concept->*f)(&_model, std::forward<Args>(args)...);
	}

	using concept = Concept;

private:
	std::aligned_storage_t<small_size> _model;
	const Concept* _concept = nullptr;
};

} // namespace poly


