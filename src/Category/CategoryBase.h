#pragma once

#include <boost/hana.hpp>
#include <functional>
#include <iostream>
#include <string_view>
#include <tuple>

#include "../Utils/MetaUtils.h"

template <class T>
constexpr std::string_view type_name() {
  using namespace std;
#ifdef __clang__
  string_view p = __PRETTY_FUNCTION__;
  return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
  string_view p = __PRETTY_FUNCTION__;
#if __cplusplus < 201402
  return string_view(p.data() + 36, p.size() - 36 - 1);
#else
  return string_view(p.data() + 49, p.find(';', 49) - 49);
#endif
#elif defined(_MSC_VER)
  string_view p = __FUNCSIG__;
  return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

template <template <typename...> typename F, typename... T>
struct template_instance {
  static constexpr bool is_instance = false;
};

template <template <typename...> typename F, typename... T>
struct template_instance<F, F<T...>> {
  static constexpr bool is_instance = true;

  template <std::size_t I>
  using get = typename std::tuple_element<I, std::tuple<T...>>::type;
};

template <template <typename...> typename F, typename... T>
constexpr bool is_template_instance_of =
    template_instance<F, F<T...>>::is_instance;

template <typename Fun, typename... Args>
constexpr decltype(auto) my_invoke(Fun &&fun, Args &&... args) {
  return std::forward<Fun>(fun)(std::forward<Args>(args)...);
}

template <typename Fun, std::size_t... Is>
constexpr decltype(auto) apply_sequence_to_impl(Fun &&fun,
                                                std::index_sequence<Is...>) {
  return my_invoke(std::forward<Fun>(fun),
                   std::integral_constant<std::size_t, Is>{}...);
}

template <size_t N, typename Fun>
constexpr decltype(auto) apply_sequence_to(Fun &&fun) {
  return apply_sequence_to_impl(std::forward<Fun>(fun),
                                std::make_index_sequence<N>{});
}

template <class T>
struct remove_cvref {
  typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

using namespace boost;

/*
 * This is an arbitrary struct for introspection purposet.
 */
struct Dummy {};

//                                 _
//   ___ ___  _ __   ___ ___ _ __ | |_ ___
//  / __/ _ \| '_ \ / __/ _ \ '_ \| __/ __|
// | (_| (_) | | | | (_|  __/ |_) | |_\__ \
//  \___\___/|_| |_|\___\___| .__/ \__|___/
//                          |_|
//

namespace concepts {

//        _     _           _
//   ___ | |__ (_) ___  ___| |_ ___
//  / _ \| '_ \| |/ _ \/ __| __/ __|
// | (_) | |_) | |  __/ (__| |_\__ \
//  \___/|_.__// |\___|\___|\__|___/
//           |__/

template <typename Obj>
constexpr bool is_universal_object() {
  return std::is_trivially_default_constructible_v<Obj>;
  // return true;
}

template <typename Obj>
constexpr bool is_universal_object(Obj const &) {
  return is_universal_object<Obj>();
}

template <typename Obj>
constexpr bool is_set_object() {
  if (is_universal_object<Obj>()) {

    auto has__is_element = hana::is_valid(
        [](auto t) -> decltype(decltype(t)::type::is_element(Dummy{})) {});

    return has__is_element(hana::type_c<Obj>);
  }
  return false;
};

template <typename Obj>
constexpr bool is_set_object(Obj const &obj) {
  return is_set_object<Obj>();
}

template <typename Obj>
constexpr bool is_type_object() {
  if (is_set_object<Obj>()) {

    auto has__type =
        hana::is_valid([](auto t) -> typename decltype(t)::type::type{});

    return has__type(hana::type_c<Obj>);
  }
  return false;
}

template <typename Obj>
constexpr bool is_type_object(Obj const &obj) {
  return is_type_object<Obj>();
}

//                             _     _
//  _ __ ___   ___  _ __ _ __ | |__ (_)___ _ __ ___  ___
// | '_ ` _ \ / _ \| '__| '_ \| '_ \| / __| '_ ` _ \/ __|
// | | | | | | (_) | |  | |_) | | | | \__ \ | | | | \__ \
// |_| |_| |_|\___/|_|  | .__/|_| |_|_|___/_| |_| |_|___/
//                      |_|

template <typename Morph>
constexpr bool is_universal_morphism() {

  auto has__source =
      hana::is_valid([](auto t) -> decltype(decltype(t)::type::source()) {});
  auto has__target =
      hana::is_valid([](auto t) -> decltype(decltype(t)::type::target()) {});

  if constexpr (has__source(hana::type_c<Morph>) &&
                has__target(hana::type_c<Morph>)) {
    return is_universal_object(Morph::source()) &&
           is_universal_object(Morph::target());
  }
  return false;
}

template <typename Morph>
constexpr bool is_universal_morphism(Morph const &morph) {
  return is_universal_morphism<Morph>();
}

template <typename Morph>
constexpr bool is_set_morphism() {
#warning "Not testing for existence of call operator. I do not know how :("
  if constexpr (is_universal_morphism<Morph>()) {
    return is_set_object(Morph::source()) && is_set_object(Morph::target());
  };
  return false;
}

template <typename Morph>
constexpr bool is_set_morphism(Morph const &morph) {
  return is_set_morphism<Morph>();
}

template <typename Morph>
constexpr bool is_type_morphism() {

  if (is_set_morphism<Morph>()) {

    auto has__source_type =
        hana::is_valid([](auto t) -> typename decltype(t)::type::source_type{});

    auto has__target_type =
        hana::is_valid([](auto t) -> typename decltype(t)::type::target_type{});

    if constexpr (has__source_type(hana::type_c<Morph>) &&
                  has__target_type(hana::type_c<Morph>)) {

      using source_type = typename Morph::source_type;
      auto has__call_operator =
          hana::is_valid([](auto t) -> decltype(hana::traits::declval(t)(
                                        std::declval<source_type>())) {});

      return bool{has__call_operator(hana::type_c<Morph>)};
    };
  }
  return false;
}

template <typename Morph>
constexpr bool is_type_morphism(Morph const &morph) {
  return is_type_morphism<Morph>();
}

} // namespace concepts

template <typename ObjX, typename ObjY>
constexpr bool is_same() {
  return std::is_same_v<ObjX, ObjY>;
}

template <typename ObjX, typename ObjY>
constexpr bool is_same(ObjX const &objX, ObjY const &objY) {
  return is_same<ObjX, ObjY>();
}

template <typename... Morphs>
constexpr bool are_composable() {

  constexpr int N = sizeof...(Morphs);

  return apply_sequence_to<N - 1>([](auto... Is) constexpr {
    using Numerics::Utils::pack_get_type;
    return (is_same(pack_get_type<Is, Morphs...>::target(),
                    pack_get_type<Is + 1, Morphs...>::source()) &&
            ...);
  });
}

template <typename... Morphs>
constexpr bool are_composable(Morphs const &...) {
  return are_composable<Morphs...>();
}

template <typename Type>
struct type_object {
  using type = Type;

  template <typename Element>
  static constexpr bool is_element(Element const &e) {
#warning                                                                       \
    "This is not good, it should keep atleast const, reference probably too."
    return std::is_same_v<remove_cvref_t<Element>, Type>;
  }
};

template <typename SourceObj, typename TargetObj>
struct universal_morphism {

  universal_morphism() {
    static_assert(concepts::is_universal_object<SourceObj>() &&
                      concepts::is_universal_object<TargetObj>(),
                  "Invalid argument");
  }

  static constexpr auto source() { return SourceObj{}; }
  static constexpr auto target() { return TargetObj{}; }
};

template <typename SourceObj, typename TargetObj, typename Fun>
struct set_morphism : public universal_morphism<SourceObj, TargetObj> {

  set_morphism(Fun _fun)
      : universal_morphism<SourceObj, TargetObj>{}
      , fun(std::move(_fun)) {

    static_assert(concepts::is_set_object<SourceObj>() &&
                      concepts::is_set_object<TargetObj>(),
                  "Invalid argument");
  }

  template <typename T>
  decltype(auto) operator()(T &&t) {

    // static_assert(SourceObj::is_element(t), "Invalid argument");

    return fun(std::forward<T>(t));
  }

private:
  Fun fun;
};

template <typename SourceObj, typename TargetObj, typename Fun>
struct type_morphism : set_morphism<SourceObj, TargetObj, Fun> {

  using source_type = typename SourceObj::type;
  using target_type = typename TargetObj::type;

  type_morphism(Fun _fun)
      : set_morphism<SourceObj, TargetObj, Fun>(std::move(_fun)) {
    static_assert(std::is_invocable_r_v<target_type, Fun, source_type>,
                  "Invalid argument");
  }
};

template <typename... Morphs>
struct composed_morphism {
protected:
  static constexpr int N = sizeof...(Morphs);
  template <std::size_t I>
  using Morph = Numerics::Utils::pack_get_type<I, Morphs...>;

  std::tuple<Morphs...> morphs;

public:
  composed_morphism(Morphs... m)
      : morphs{std::move(m)...} {
    static_assert(are_composable(m...), "Morphisms are not composable!");
  }

  static constexpr auto source() { return Morph<0>::source(); }
  static constexpr auto target() { return Morph<N - 1>::target(); }
};

template <typename... Morphs>
struct composed_universal_morphism : public composed_morphism<Morphs...> {
protected:
  using Base = composed_morphism<Morphs...>;

public:
  composed_universal_morphism(Morphs... m)
      : Base{std::move(m)...} {
    static_assert((concepts::is_universal_morphism(m) && ...),
                  "Arguments are not universal morphisms!");
  }
};

template <typename... Morphs>
struct composed_set_morphism : public composed_universal_morphism<Morphs...> {
protected:
  using Base = composed_universal_morphism<Morphs...>;

public:
  composed_set_morphism(Morphs... m)
      : Base{std::move(m)...} {
    static_assert((concepts::is_set_morphism(m) && ...),
                  "Arguments are not set morphisms!");
  }

  template <typename T>
  decltype(auto) operator()(T &&t) {

    // static_assert(source().is_element(t), "Invalid argument");

    return eval<0>(std::forward<T>(t));
  }

private:
  template <int I, typename T>
  decltype(auto) eval(T &&t) {
    if constexpr (I == (Base::N - 1)) {
      return std::get<I>(Base::morphs)(std::forward<T>(t));
    } else {
      return eval<I + 1>(std::get<I>(Base::morphs)(std::forward<T>(t)));
    }
  }
};

template <typename... Morphs>
struct composed_type_morphism : public composed_set_morphism<Morphs...> {
protected:
  using Base = composed_set_morphism<Morphs...>;

public:
  composed_type_morphism(Morphs... m)
      : Base{std::move(m)...} {
    static_assert((concepts::is_type_morphism(m) && ...),
                  "Arguments are not type morphisms!");
  }

  using source_type = typename decltype(Base::source())::type;
  using target_type = typename decltype(Base::target())::type;
};

// template <typename... Objs>
// struct set_product {

//   template <typename ProdElement>
//   static constexpr bool is_element(ProdElement const &e) {

//     if constexpr (is_template_instance_of<std::tuple, ProdElement>) {

//       using TI    = template_instance<std::tuple, ProdElement>;
//       using Tuple = std::tuple<Objs...>;

//       return apply_sequence_to<sizeof...(Objs)>([&e](auto... Is) {

// #warning                                                                       \
//     "This is not good because std::get<Is>(e) propagates cvref to the result"
//         return (std::tuple_element_t<Is, Tuple>::is_element(std::get<Is>(e))
//         &&
//                 ...);
//       });
//     }
//     return false;
//   }
// };

/*
 * `universal_category` completely general category
 *
 * Objects: anything is an object in this category
 *   requirement: Object is represented by a type. All information has to be
 * stored in the type but is usually passed around as a constexpr value. It is
 * inspired by `boost::hana::basic_type<T>`.
 *
 * Morphism: represents and arrow between a source object and a target object
 *   requirement:
 *        1. `static constexpr auto source()` returns source object
 *        2. `static constexpr auto target()` returns target object
 *
 */
struct universal_category {

  template <template <typename...> typename T>
  struct template_object {};

  template <typename Obj>
  static constexpr bool is_object(Obj const &) {
    return std::is_trivially_default_constructible_v<Obj>;
  }

  template <typename Morph>
  static constexpr bool is_morphism(Morph const &morph) {
    return concepts::is_universal_morphism(morph);
  }

  template <typename... Morphs>
  static constexpr auto compose(Morphs &&... morphs) {
    return composed_universal_morphism<std::decay_t<Morphs>...>{
        std::forward<Morphs>(morphs)...};
  }
};

struct set_category {

  template <typename Obj>
  static constexpr bool is_object(Obj const &obj) {
    return concepts::is_set_object(obj);
  }

  template <typename Morph>
  static constexpr bool is_morphism(Morph const &morph) {
    return concepts::is_set_morphism(morph);
  }

  template <typename... Morphs>
  static constexpr auto compose(Morphs &&... morphs) {
    return composed_set_morphism<std::decay_t<Morphs>...>{
        std::forward<Morphs>(morphs)...};
  }
};

////////////////////////////////////////////////////////////////////////
//  _____                             _                               //
// |_   _|   _ _ __   ___    ___ __ _| |_ ___  __ _  ___  _ __ _   _  //
//   | || | | | '_ \ / _ \  / __/ _` | __/ _ \/ _` |/ _ \| '__| | | | //
//   | || |_| | |_) |  __/ | (_| (_| | ||  __/ (_| | (_) | |  | |_| | //
//   |_| \__, | .__/ \___|  \___\__,_|\__\___|\__, |\___/|_|   \__, | //
//       |___/|_|                             |___/            |___/  //
//                                                                    //
////////////////////////////////////////////////////////////////////////

struct type_category {

  template <typename Type>
  using object = type_object<Type>;

  template <typename SourceType, typename TargetType, typename Fun>
  using morphism = type_morphism<SourceType, TargetType, Fun>;

  template <typename Obj>
  static constexpr bool is_object(Obj const &obj) {
    return concepts::is_type_object(obj);
  }

  template <typename Morph>
  static constexpr bool is_morphism(Morph const &morph) {
    return concepts::is_type_morphism(morph);
  }

  template <typename... Morphs>
  static constexpr auto compose(Morphs &&... morphs) {
    return composed_type_morphism<std::decay_t<Morphs>...>{
        std::forward<Morphs>(morphs)...};
  }

  // struct product /* multi_functor_concept */ {
  //   template <int I> using source_category = type_category;
  //   using target_category                  = type_category;

  //   template <typename... Objs>
  //   static constexpr auto map(Objs const &... objs) {

  //     static_assert((is_object(objs) && ...), "Invalid arguments!");

  //     return object<std::tuple<typename Objs::type...>>{};
  //   }

  //   /* canonical projection
  //    */
  //   template <std::size_t I, typename ProductObj>
  //   static auto pi(ProductObj const &obj) {

  //     static_assert(is_product(obj), "Invalid argument!");

  //     using Src_t = typename ProductObj::type;
  //     using Trg_t = typename template_instance<
  //         std::tuple, typename ProductObj::type>::template get<I>;

  //     return morphism{[](auto &&s) -> decltype(auto) {

  //                       static_assert(std::is_same_v<Src_t, decltype(s)>,
  //                                     "Invalid argument");

  //                       return std::get<I>(std::forward<decltype(s)>(s));
  //                     },
  //                     object<Src_t>{}, object<Trg_t>{}};
  //   }

  //   // template <typename... Morphs>
  //   // static auto fmap(Morphs &&... morphs) {

  //   //   static_assert((is_morphism(morphs) && ...), "Invalid arguments!");

  //   //   constexpr int N = sizeof...(morphs);

  //   //   using SrcObj = decltype(map(morphs.source()...));
  //   //   using TrgObj = decltype(map(morphs.target()...));
  //   //   using Src_t  = typename SrcObj::type;
  //   //   using Trg_t  = typename TrgObj::type;

  //   //   return morphism{
  //   //       [morphs...](Src_t &&s) mutable -> decltype(auto) {

  //   //         auto morp = [&](auto I) { return pack_get<I>(morphs...); };
  //   //         auto proj = [&](auto I) { return pi<I>(object<Src_t>{}); };
  //   //         auto fun  = [&](auto I) { return compose(morp(I), proj(I));
  //   };

  //   //         auto impl = [&](auto... I) {
  //   //           return std::make_tuple(fun(I)(std::forward<Src_t>(s))...);
  //   //         };

  //   //         return std::apply(impl, integral_sequence<N>);
  //   //       },
  //   //       object<Src_t>{}, object<Trg_t>{}};
  //   // }
  // };
}; // namespace type_category

// struct function_object_category {

//   template <typename SourceType, typename TargetType> struct object {
//     static constexpr auto source() {
//       return type_category::object<SourceType>{};
//     }
//     static constexpr auto target() {
//       return type_category::object<TargetType>{};
//     }

//     template <typename T> static constexpr bool is_element(T const &) {
//       using TI = template_instance<type_category::morphism, T>;
//       if constexpr (TI::is_instance) {
//         return std::is_same_v<typename TI::template get<0>, SourceType> &&
//                std::is_same_v<typename TI::template get<1>, TargetType>;
//       } else {
//         return false;
//       }
//     }
//   };

//   template <typename T> static constexpr bool is_object(T) {
//     if constexpr (is_template_instance_of<object, T>) {
//       return true;
//     } else {
//       return false;
//     }
//   }
// };
