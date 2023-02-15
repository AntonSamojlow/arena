#include <type_traits>
#include <utility>

namespace tools {

/// unique_resource is a universal RAII wrapper for resource handles. Typically, such resource handles are of trivial
/// type and come with a factory function and a clean-up or deleter function that do not throw exceptions. The clean-up
/// function together with the result of the creation function is used to create a unique_resource variable, that on
/// destruction will call the clean-up function. Access to the underlying resource handle is achieved through get() and
/// in case of a pointer type resource through a set of convenience pointer operator functions
/// --> reference: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4873.html#scopeguard.uniqueres
template <class R, class D>
class unique_resource {
 private:
	using R1 = std::conditional_t<std::is_reference_v<R>, std::reference_wrapper<std::remove_reference_t<R>>, R>;
	R1 resource_;
	D deleter_;
	bool execute_on_reset_{true};

 public:
	// constructors
	unique_resource() = default;
	template <class RR, class DD>
		requires std::is_constructible_v<D, DD> &&
						 (std::is_nothrow_constructible_v<R1, RR> || std::is_constructible_v<R1, RR&>) &&
						 (std::is_nothrow_constructible_v<D, DD> || std::is_constructible_v<D, DD&>)

	unique_resource(RR&& res, DD&& del) noexcept {
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
	unique_resource(unique_resource&& rhs) noexcept { --TODO-- }

	// destructor
	~unique_resource();

	// assignment
	auto operator=(unique_resource&& rhs) noexcept -> unique_resource&;

	// other member functions
	auto reset() noexcept -> void;
	template <class RR>
	auto reset(RR&& new_value) -> void;
	auto release() noexcept -> void { execute_on_reset_ = false; }
	auto get() const noexcept -> const R& { return resource_; }
	auto operator*() const noexcept { return *get(); }
	auto operator->() const noexcept -> R { return resource_; }
	auto get_deleter() const noexcept -> const D& { return deleter_; }
};

// deduction guide
template <class R, class D>
unique_resource(R, D) -> unique_resource<R, D>;

// assignment
template <class R, class D>
auto unique_resource<R, D>::operator=(unique_resource<R, D>&& rhs) noexcept -> unique_resource<R, D>& {
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
}

// other member functions
template <class R, class D>
auto unique_resource<R, D>::reset() noexcept -> void {
	if (execute_on_reset_) {
		execute_on_reset_ = false;
		deleter(resource_);
	}
}

template <class R, class D>
template <class RR>
auto unique_resource<R, D>::reset(RR&& new_value) -> void {
	reset();
	if constexpr (std::is_nothrow_assignable_v<R&, RR>) {
		resource_ = std::forward<RR>(new_value);
	} else {
		resource_ = as_const(new_value);
	}
	execute_on_reset_ = true;
}

}  // namespace tools