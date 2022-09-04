<hr/>

<div align="center"> 
    <img src="assets/logo.png" height=312>
</div>

<br/>

<p align="center">
    A thread-safe C++17 Event Library
</p>

<hr/>

<div align="center">

### Description

<div align="left">

_Ereignis_ is a library that implements an easy to use Event/Callback mechanism.  
With _Ereignis_ it's possible to (un)register callbacks to an `event_manager`, clear all the registered callbacks and also `fire` a specific event, which in turn will invoke all registered callbacks.  
When the specified callback does not return `void` it is possible to iteratively invoke the callbacks and receive their result.

</div>

### Installation

<div align="left">

- With FetchContent
  ```cmake
  include(FetchContent)

  FetchContent_Declare(ereignis GIT_REPOSITORY "https://github.com/Soundux/ereignis")
  FetchContent_MakeAvailable(ereignis)

  target_link_libraries(<target> ereignis)
  ```
- As Git-Submodule
  ```bash
  $ git clone https://github.com/Soundux/ereignis
  ```
  ```cmake
  add_subdirectory(ereignis)
  target_link_libraries(<target> ereignis)
  ```


</div>

<br>

### Usage examples


<div align="left">

* Basic callback
  ```cpp
  #include <ereignis/manager.hpp>

  using ereignis::event_manager;
  using ereignis::event;

  auto manager = event_manager<
    event<0, void(int)>
  >;

  manager.at<0>().add([](int i) { std::cout << i << std::endl; });
  manager.at<0>().fire(1337);
  ```

* Result iteration
  ```cpp
  #include <ereignis/manager.hpp>

  enum class window_event
  {
    close
  };

  using ereignis::event_manager;
  using ereignis::event;

  auto manager = event_manager<
    event<window_event::close, bool()>
  >;

  manager.at<window_event::close>().add([]() -> bool { return true; });
  
  manager.at<window_event::close>().add([]() -> bool 
  { 
    std::cout << "Reached!" << std::endl; 
    return false; 
  });

  // Lets fire all callbacks until we get `true`.

  for (const auto& result : manager.at<window_event::close>().fire())
  {
    if (result)
    {
      return;
    }
  }

  // 'Reached!' will never be printed.
  ```

</div>