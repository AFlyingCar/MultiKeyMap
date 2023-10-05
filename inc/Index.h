#ifndef INDEX_H
# define INDEX_H

# include <tuple>
# include <type_traits>

template<typename T = void>
struct RemoveCVRef {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<typename T = void>
using RemoveCVRef_t = typename RemoveCVRef<T>::type;

///////

template<typename T, typename... Ts>
struct IndexImpl;

template<typename T, typename... Ts>
struct IndexImpl<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template<typename T, typename U, typename... Ts>
struct IndexImpl<T, U, Ts...> : std::integral_constant<std::size_t, 1 + IndexImpl<T, Ts...>::value> {};

template<typename T, typename... Ts>
struct Index: IndexImpl<RemoveCVRef_t<T>, Ts...> {};

template<typename T, typename... Ts>
constexpr std::size_t Index_v = Index<T, Ts...>::value;

template<typename T, typename... Ts>
constexpr std::size_t getIndexOfType() {
    return Index<T, Ts...>::value;
}

///////

template<size_t... Is, typename Tuple, typename F>
void forEach(std::index_sequence<Is...>, Tuple&& tuple, F&& f) {
    int unused[] = { 0, ( (void)f(std::get<Is>(std::forward<Tuple>(tuple))), 0 )... };
    (void)unused;
}

template<typename F, typename... Args>
void forEach(std::tuple<Args...>& tuple, F&& f) {
    forEach(std::make_index_sequence<sizeof...(Args)>{}, tuple, f);
}

#endif

