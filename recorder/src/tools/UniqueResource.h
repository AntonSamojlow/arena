#include <type_traits>
#include <utility>

namespace tools {
/// wrapper around a resource, basically copied from:
/// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4873.html#scopeguard.uniqueres
///
/// unique_resource is a universal RAII wrapper for resource handles. Typically, such resource handles are of trivial
/// type and come with a factory function and a clean-up or deleter function that do not throw exceptions. The clean-up
/// function together with the result of the creation function is used to create a unique_resource variable, that on
/// destruction will call the clean-up function. Access to the underlying resource handle is achieved through get() and
/// in case of a pointer type resource through a set of convenience pointer operator functions
template <class R, class D>
class unique_resource {
 private:
	using R1 = std::conditional_t<std::is_reference_v<R>, std::reference_wrapper<std::remove_reference_t<R>>, R>;
	R1 resource_;
	D deleter_;
	bool execute_on_reset_{true};

 public:
	// constructors
	unique_resource()
		requires std::is_default_constructible_v<R> && std::is_default_constructible_v<D>
			: resource_({}), deleter_({}), execute_on_reset_(false) {}

	template <class RR, class DD>
		requires std::is_constructible_v<R1, RR> && std::is_constructible_v<D, DD> &&
						 (std::is_nothrow_constructible_v<R1, RR> || std::is_constructible_v<R1, RR&>) &&
						 (std::is_nothrow_constructible_v<D, DD> || std::is_constructible_v<D, DD&>)
	unique_resource(RR&& res, DD&& del) noexcept(
		(std::is_nothrow_constructible_v<R1, RR> || std::is_constructible_v<R1, RR&>)&&(
			std::is_nothrow_constructible_v<D, DD> || std::is_constructible_v<D, DD&>)) {
		try {
			if constexpr (std::is_nothrow_constructible_v<R1, RR>) {
				resource_ = std::forward<RR>(res);
			} else {
				resource_ = res;
			}
		} catch (...) {
			del(res);
		}

		try {
			if constexpr (std::is_nothrow_constructible_v<D, DD>) {
				deleter_ = std::forward<DD>(del);
			} else {
				deleter_ = del;
			}
		} catch (...) {
			del(resource_);
		}
	}

	unique_resource(unique_resource const&) = delete;

	unique_resource(unique_resource&& rhs) noexcept(
		std::is_nothrow_move_constructible_v<R1>&& std::is_nothrow_move_constructible_v<D>) {
		// If initialization of resource throws an exception, rhs is left owning the resource and will free it in due time:
		if constexpr (std::is_nothrow_move_constructible_v<R1>) {
			resource_ = std::move(rhs.resource_);
		} else {
			resource_ = rhs.resource_;
		}
		try {
			if constexpr (std::is_nothrow_move_constructible_v<D>) {
				deleter_ = std::move(rhs.deleter_);
			} else {
				deleter_ = rhs.deleter_;
			}
		} catch (...) {
			rhs.deleter_(resource_);
			rhs.release();
		}
		execute_on_reset_ = std::exchange(rhs.execute_on_reset_, false);
	}

	// destructor
	~unique_resource() { reset(); }

	// assignments
	auto operator=(unique_resource const&) -> unique_resource& = delete;

	auto operator=(unique_resource&& rhs) noexcept(
		std::is_nothrow_move_assignable_v<R1>&& std::is_nothrow_move_assignable_v<D>) -> unique_resource& {
		// If a copy of a member throws an exception, this mechanism leaves rhs intact and *this in the released state:
		reset();
		if constexpr (std::is_nothrow_move_assignable_v<R1>) {
			if constexpr (std::is_nothrow_move_assignable_v<D>) {
				resource_ = std::move(rhs.resource_);
				deleter_ = std::move(rhs.deleter_);
			} else {
				deleter_ = rhs.deleter_;
				resource_ = std::move(rhs.resource_);
			}
		} else {
			if constexpr (std::is_nothrow_move_assignable_v<D>) {
				resource_ = rhs.resource_;
				deleter_ = std::move(rhs.deleter_);
			} else {
				resource_ = rhs.resource_;
				deleter_ = rhs.deleter_;
			}
		}
		execute_on_reset_ = std::exchange(rhs.execute_on_reset_, false);
		return *this;
	}

	// other member functions
	auto reset() noexcept -> void {
		if (execute_on_reset_) {
			execute_on_reset_ = false;
			deleter_(resource_);
		}
	}

	template <class RR>
	auto reset(RR&& new_value) -> void {
		reset();
		if constexpr (std::is_nothrow_assignable_v<R&, RR>) {
			resource_ = std::forward<RR>(new_value);
		} else {
			resource_ = std::as_const(new_value);
		}
		execute_on_reset_ = true;
	}

	auto release() noexcept -> void { execute_on_reset_ = false; }

	auto get() const noexcept -> const R& { return resource_; }

	auto operator*() const noexcept -> std::add_lvalue_reference_t<std::remove_pointer_t<R>>
		requires std::is_pointer_v<R> && (not std::is_void_v<std::remove_pointer_t<R>>)
	{
		return *get();
	}

	auto operator->() const noexcept -> R
		requires std::is_pointer_v<R>
	{
		return resource_;
	}

	auto get_deleter() const noexcept -> const D& { return deleter_; }
};

// deduction guide
template <class R, class D>
unique_resource(R, D) -> unique_resource<R, D>;

/// This creation function exists to avoid calling a deleter function with an invalid argument.
/// Any failure during construction of the return value will not call del(resource) if resource == invalid.
/// Example: auto file = make_unique_resource_checked(
///   ::fopen("potentially_nonexistent_file.txt", "r"), nullptr, [](auto fptr){ ::fclose(fptr); });
template <class R, class D, class S = std::decay_t<R>>
auto make_unique_resource_checked(R&& resource, const S& invalid, D&& del) noexcept(
	std::is_nothrow_constructible_v<std::decay_t<R>, R>&& std::is_nothrow_constructible_v<std::decay_t<D>, D>)
	-> unique_resource<std::decay_t<R>, std::decay_t<D>> {
	try {
		return {std::forward(resource), std::forward(del), resource != invalid};
	} catch (...) {
		if (resource != invalid) {
			del(resource);
		}
	}
}

}  // namespace tools
