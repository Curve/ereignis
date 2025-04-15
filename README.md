
<div align="center"> 
    <img src="assets/logo.png" height=312>
</div>

<br/>

<p align="center">
    A thread-safe C++23 Event Library
</p>

<hr/>

## 📖 Description

_Ereignis_ is a library that implements an easy to use Event/Callback mechanism that allows for lazy-evaluation of the the results returned by `fire`.

## 📦 Installation

> [!NOTE]  
> This library requires a C++23 capable compiler!
> * See versions `< 5.0` for C++20 support.
> * See versions `< 2.0` for C++17 support.

* Using [CPM](https://github.com/cpm-cmake/CPM.cmake)
  ```cmake
  CPMFindPackage(
    NAME           ereignis
    VERSION        5.0
    GIT_REPOSITORY "https://github.com/Curve/ereignis"
  )
  ```

* Using FetchContent
  ```cmake
  include(FetchContent)

  FetchContent_Declare(ereignis GIT_REPOSITORY "https://github.com/Curve/ereignis" GIT_TAG v5.0)
  FetchContent_MakeAvailable(ereignis)

  target_link_libraries(<target> cr::ereignis)
  ```

## 📃 Usage

* Basic callback

  ```cpp
  #include <ereignis/manager/manager.hpp>

  using ereignis::event;

  ereignis::manager<
    event<0, void(int)>
  > manager;

  manager.get<0>().add([](int i) { std::cout << i << std::endl; });
  manager.get<0>().fire(1337);
  ```

* Result iteration

  ```cpp
  #include <ereignis/manager/manager.hpp>

  enum class window_event
  {
    close
  };

  using ereignis::event;

  ereignis::manager<
    event<window_event::close, bool()>
  > manager;

  manager.get<window_event::close>().add([]() -> bool { return true; });
  
  manager.get<window_event::close>().add([]() -> bool 
  { 
    std::cout << "Reached!" << std::endl; 
    return false; 
  });

  // Lets fire all callbacks until we get `true`.

  manager.get<window_event::close>().fire().find(true);

  // or...

  for (const auto& result : manager.get<window_event::close>().fire())
  {
    if (result)
    {
      return;
    }
  } 

  // 'Reached!' will never be printed.
  ```
