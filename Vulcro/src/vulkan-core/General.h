#pragma once

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include <iostream>
#include <functional>
#include <memory>
#include <unordered_map>

//using namespace std;

using std::vector;

template<typename T>
using temps = std::initializer_list<T> &&;

using std::shared_ptr;
using std::make_shared;
//using chrono = std::chrono;
using std::function;

//using namespace glm;

using glm::uint32_t;
using glm::uint;
using glm::uint64_t;
using glm::int8_t;
using glm::uint8_t;

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::int32_t;

#define VULCRO_DONT_COPY(type) type(const type&) = delete;

