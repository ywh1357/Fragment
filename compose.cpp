#include <botan/base64.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <type_traits>
#include <tuple>

class filter_a {
public:
	std::string hello(float prev) const {
		fmt::print("filter_a hello {} + 1\n", prev);
		return fmt::format("{}", prev + 1);
	}
};

class filter_b {
public:
	int hello(const std::string& prev) const {
		fmt::print("filter_b hello {} + 2\n", prev);
		return std::stoi(prev) + 2;
	}
};

class filter_c {
public:
	std::string hello(const std::vector<uint8_t>& bytes) const {
		auto str = std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		fmt::print("filter_c hello {}\n", str);
		return str;
	}
};

using TupleType = std::tuple<filter_a, filter_b, filter_c>;
using ReverseTupleType = std::tuple<filter_b, filter_a>;

template <class F, class G>
auto compose(F f, G g) {
	return [f, g](auto x) { return f(g(x)); };
}

template <class F, typename... Fs>
auto compose(F f, Fs&& ... fs) {
	return compose(f, compose(fs...));
}

template <typename F, typename Tuple, std::size_t... I>
auto compose_filters_impl(const Tuple& filters, F fn, std::index_sequence<I...>) {
	return compose(fn(std::get<I>(filters))...);
}

template <typename F, template <typename...> class Tuple, typename... FilterTypes>
auto compose_filters(const Tuple<FilterTypes...>& filters, F fn) {
	return compose_filters_impl(filters, fn, std::make_index_sequence<sizeof...(FilterTypes)>());
}

int main() {

	TupleType filters({}, {}, {});
	// foreach_filter_reverse(filters);
	auto run = compose_filters(
		filters, 
		[](const auto& filter) { 
			return [=](auto x) { 
				return filter.hello(x); 
			}; 
		}
	);

	std::string input_msg = "1";
	std::vector<uint8_t> input_vec{ input_msg.begin(), input_msg.end() };

	auto x = run(input_vec);
	fmt::print("x = {}\n", x);

	return 0;
}
