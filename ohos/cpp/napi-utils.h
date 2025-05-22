
#include <arm_neon.h>
#include <napi/native_api.h>
#include <cstdint>
#include <string>






#define RETURN_IF_NULLOPT_OR_ASSIGN( recv, expr) \
    if (auto val = (expr); !val) {\
        return nullptr; \
    } else { \
        (recv) = *val; \
    }

#define NAPI_RETURN_IF_NOT_OK(expr) \
    if (auto napiCallResult = (expr); napiCallResult != napi_ok) { \
    }

#define GET_NAPI_ARG_OR_RETURN(type, var, env, arg) \
    type var{}; \
	RETURN_IF_NULLOPT_OR_ASSIGN(var, NapiUtils::getValue<type>(env, arg)); \
   

#define GET_NAPI_ARG(type, var, env, arg) \
    constexpr const char *__funcname_define_##var = __PRETTY_FUNCTION__; \
    GET_NAPI_ARG_OR_RETURN(type, var, env, arg);

#define GET_NAPI_ARGS(env, info, cout) \
    constexpr const char *__funcname_setup_args = __PRETTY_FUNCTION__; \
   	size_t argc; \
    NAPI_RETURN_IF_NOT_OK(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr)); \
    if (argc != (cout)) { \
        return nullptr; \
    } \
    napi_value args[(cout)] = { nullptr }; \
    NAPI_RETURN_IF_NOT_OK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

struct NapiUtils {
public:
    template<typename T>
 	static std::optional<T> getValue(napi_env env, napi_value value);
	
  	template<>
    std::optional<int32_t> getValue(napi_env env, napi_value value) {
        int32_t i;
        NAPI_RETURN_IF_NOT_OK(napi_get_value_int32(env, value, &i));
        return i;
    }
	template<>
    std::optional<float32_t> getValue(napi_env env, napi_value value) {
        double  i;
        NAPI_RETURN_IF_NOT_OK(napi_get_value_double(env, value, &i));
        return (float32_t)i;
    }
  	template<>
    std::optional<std::string> getValue(napi_env env, napi_value value) {
        size_t length;
        NAPI_RETURN_IF_NOT_OK( napi_get_value_string_latin1(env, value, nullptr, 0, &length));
        std::string str(length, '\0');
        NAPI_RETURN_IF_NOT_OK( napi_get_value_string_latin1(env, value, str.data(), str.capacity(), nullptr));
        return str;
    }
};