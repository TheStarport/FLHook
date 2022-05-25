@page page_contents Contents
@tableofcontents
@subpage page_intro Intro
@subpage page_install Install
@subpage page_system System

Freelancer was built using Visual C++ 6.0 libraries. A few functions in Freelancer's API use STL classes (e.g. `std::string`, `std::vector`, etc.). Unfortunately, binary compatibility between VC6 and modern compilers was not preserved, and as a result interoperability is not possible.

In order to circumvent this issue, FLHook (as of version 2.1) includes a minimal reimplementation of the VC6 STL called `st6`. Affected API functions have had their signatures modified to invoke `st6` instead of `std`. Most `std` features should "just work" on `st6` classes, though explicit conversion is required to switch between libraries (e.g transforming a `st6::map` into a `std::map` or vice-versa requires rebuilding the map).