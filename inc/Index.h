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

///////

template<typename... Ts>
using TupleCat_t = decltype(std::tuple_cat(std::declval<Ts>()...));

/**
 * @brief Removes all instances of type T from parameter pack Ts
 * @details https://stackoverflow.com/a/23863962
 */
template<typename T, typename... Ts>
using RemoveType = TupleCat_t<
    typename std::conditional<
        std::is_same<T, Ts>::value,
        std::tuple<>,
        std::tuple<Ts>
    >::type...
>;

///////
// Following adapted from: https://stackoverflow.com/a/24046437

template<std::size_t I, std::size_t... Is, typename Tuple>
auto tupleTailImpl(std::index_sequence<Is...>, Tuple&& t) {
    // Shift all indices up by I so we only get the last "num Is" - I elements
    return std::make_tuple(std::get<Is+I>(t)...);
}

template<std::size_t I, typename... Ts>
auto tupleTail(std::tuple<Ts...>&& t) {
    // Only get "num Ts" - I elements
    return tupleTailImpl<I>(std::make_index_sequence<sizeof...(Ts) - I>(), t);
}

template<std::size_t I, typename... Ts>
auto tupleTail(std::tuple<Ts...>& t) {
    // Only get "num Ts" - I elements
    return tupleTailImpl<I>(std::make_index_sequence<sizeof...(Ts) - I>(), t);
}

template<std::size_t... Is, typename Tuple>
auto tupleHeadImpl(std::index_sequence<Is...>, Tuple&& t) {
    return std::make_tuple(std::get<Is>(t)...);
}

template<std::size_t I, typename... Ts>
auto tupleHead(std::tuple<Ts...>&& t) {
    // Only get "num Ts" - I elements
    return tupleHeadImpl(std::make_index_sequence<sizeof...(Ts) - I - 1U>(), t);
}

template<std::size_t I, typename... Ts>
auto tupleHead(std::tuple<Ts...>& t) {
    // Only get "num Ts" - I elements
    return tupleHeadImpl(std::make_index_sequence<sizeof...(Ts) - I - 1U>(), t);
}

template<std::size_t I, typename... Ts>
using TupleHead = decltype(tupleHead<I, Ts...>(std::declval<std::tuple<Ts...>>()));

template<std::size_t I, typename... Ts>
using TupleTail = decltype(tupleTail<I, Ts...>(std::declval<std::tuple<Ts...>>()));

template<std::size_t I, typename... Ts>
auto splitTuple(std::tuple<Ts...>&& t) {
    return std::make_pair(
        tupleHead<I + 1, Ts...>(t), // Split 1 past the index so we don't accidentally duplicate the split location
        tupleTail<I, Ts...>(t)
    );
}

template<std::size_t I, typename... Ts>
auto splitTuple(std::tuple<Ts...>& t) {
    return std::make_pair(
        tupleHead<I + 1, Ts...>(t), // Split 1 past the index so we don't accidentally duplicate the split location
        tupleTail<I, Ts...>(t)
    );
}

#endif

