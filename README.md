<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/logo-dark.svg">
    <img src="assets/logo-light.svg" width="600">
  </picture>
</p>

## 📃 Description

_Ereignis_ is a C++23 library that implements an easy to use Event / Callback mechanism.
It also allows for lazy-evaluation of the callback results and supports awaiting `void` returning events.

## 📦 Installation

> [!NOTE]  
> This library requires a C++23 capable compiler!
> * See `< 5.0` for C++20 support
> * See `< 2.0` for C++17 support

* Using [CPM](https://github.com/cpm-cmake/CPM.cmake)
  ```cmake
  CPMFindPackage(
    NAME           ereignis
    VERSION        5.0.0
    GIT_REPOSITORY "https://github.com/Curve/ereignis"
  )
  ```

* Using FetchContent
  ```cmake
  include(FetchContent)

  FetchContent_Declare(ereignis GIT_REPOSITORY "https://github.com/Curve/ereignis" GIT_TAG v5.0.0)
  FetchContent_MakeAvailable(ereignis)

  target_link_libraries(<target> cr::ereignis)
  ```

## 📋 Documentation

### `manager<Events...>`

A manager for multiple events.

```cpp
#include <ereignis/manager/manager.hpp>

ereignis::manager<
  ereignis::event<0, void(int)>,
  ereignis::event<1, bool(int)>
> manager;

// Get Event Type
static_assert(std::same_as<decltype(manager)::event<0>, ereignis::event<0, void(int)>>);

// Get Event
manager.get<0>().fire(10);

// Clear / Remove Events without compile-time id
manager.clear(0);
manager.remove(0, ...);
```

### `event<Id, Signature>`

Manages a single event and associated callbacks.

```cpp
enum class event_id
{
  one,
};

auto event = ereignis::event<event_id::one, bool(int)>{};

// Keep ID in case you want to remove event later 
auto id = event.add([](int) { return false; });

// Once fired, this event is deleted
event.once([](int) { return false; });

// Fire event. In case the Signature does not return void, you need to iterate over the results to invoke the callbacks

for (auto result : event.fire(10))
{
  // ...
}

// Fire until true is received
std::optional<bool> result = event.fire(10).find(true);

// Fire until something else than true is received
std::optional<bool> result = event.fire(10).skip(true);
```
