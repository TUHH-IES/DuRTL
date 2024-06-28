//
// Created by gianluca on 18.04.23.
//

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/type_traits/is_detected.hpp>

#include <concepts>
#include <filesystem>
#include <string_view>
#include <type_traits>

template<class T>
concept string_like = std::is_convertible_v<T, std::string_view>;

template<class T>
concept filesystem_like = std::is_same_v<T, std::filesystem::path> || std::is_same_v<T, boost::filesystem::path>;

template<class T>
using begin_non_mf_t = decltype(begin(std::declval<T>()));
template<class T>
using begin_mf_t = decltype(std::declval<T>().begin());
template<class T>
using begin_t = decltype(T::begin);
template<class T>
using end_non_mf_t = decltype(end(std::declval<T>()));
template<class T>
using end_mf_t = decltype(std::declval<T>().end());
template<class T>
using end_t = decltype(T::end);

template<class T>
constexpr bool has_member_begin_or_end{
    boost::is_detected_v<begin_mf_t, T> ||
    boost::is_detected_v<begin_t, T> ||
    boost::is_detected_v<end_mf_t, T> ||
    boost::is_detected_v<end_t, T>};

template<class T>
std::add_lvalue_reference_t<T> declref() noexcept;
template<class T>
using declref_t = decltype(declref<T>());

template<class T>
concept range_like =
    (requires /*Arrays*/
     {
       requires std::is_array_v<T>;
       requires std::extent_v<T> != 0;// Extent is known.
     }) ||
    (/*Classes with member begin/end*/
     requires {
       requires std::is_class_v<T> && has_member_begin_or_end<T>;
     } &&
     requires(begin_mf_t<declref_t<T>> _begin,
              end_mf_t<declref_t<T>> _end) {
       { _begin != _end } -> std::same_as<bool>;
       { *_begin } -> std::convertible_to<typename T::value_type>;
       { ++_begin };
     }) ||
    (/*Types with non-member begin/end*/
     requires {
       requires !std::is_class_v<T> || !has_member_begin_or_end<T>;
     } &&
     requires(begin_non_mf_t<declref_t<T>> _begin,
              end_non_mf_t<declref_t<T>> _end) {
       { _begin != _end } -> std::same_as<bool>;
       { *_begin } -> std::convertible_to<typename T::value_type>;
       { ++_begin };
     });