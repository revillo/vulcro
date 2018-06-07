#pragma once

#include "glm/common.hpp"
#include <vector>
#include <iostream>
#include <functional>
#include <memory>

//using namespace std;

using std::vector;

template<typename T>
using temps = std::initializer_list<T> &&;

using std::shared_ptr;
using std::make_shared;
//using chrono = std::chrono;
using std::function;

using namespace glm;

#define VULCRO_DONT_COPY(type) type(const type&) = delete;