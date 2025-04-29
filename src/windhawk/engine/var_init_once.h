#pragma once

// The following macros are used to initialize static variables once in a
// thread-safe manner while avoiding TLS, which is what MSVC uses for static
// variables.

// Similar to:
// static T var_name(...);
#define STATIC_INIT_ONCE(T, var_name, ...)                                 \
    T* var_name;                                                           \
    do {                                                                   \
        static alignas(T) char static_init_once_storage_[sizeof(T)];       \
        static std::once_flag static_init_once_flag_;                      \
        std::call_once(static_init_once_flag_, []() {                      \
            new (static_init_once_storage_) T(__VA_ARGS__);                \
            if constexpr (!std::is_trivially_destructible_v<T>) {          \
                std::atexit([]() {                                         \
                    reinterpret_cast<T*>(static_init_once_storage_)->~T(); \
                });                                                        \
            }                                                              \
        });                                                                \
        var_name = reinterpret_cast<T*>(static_init_once_storage_);        \
    } while (0)

// Similar to:
// static T var_name = initializer;
#define STATIC_INIT_ONCE_TRIVIAL(T, var_name, initializer)  \
    static constinit T var_name;                            \
    do {                                                    \
        static_assert(std::is_trivially_destructible_v<T>); \
        static std::once_flag static_init_once_flag_;       \
        std::call_once(static_init_once_flag_,              \
                       []() { var_name = initializer; });   \
    } while (0)

// Similar to:
// static T ptr = (T)GetProcAddress(GetModuleHandle(module_name), proc_name);
#define GET_PROC_ADDRESS_ONCE(T, ptr, module_name, proc_name)          \
    static T ptr;                                                      \
    do {                                                               \
        static_assert(std::is_trivially_destructible_v<T>);            \
        static std::once_flag get_proc_address_once_flag_;             \
        std::call_once(get_proc_address_once_flag_, []() {             \
            HMODULE get_proc_address_once_module_ =                    \
                GetModuleHandle(module_name);                          \
            if (get_proc_address_once_module_) {                       \
                ptr = (T)GetProcAddress(get_proc_address_once_module_, \
                                        proc_name);                    \
            }                                                          \
        });                                                            \
    } while (0)

// Similar to:
// static T ptr =
//     (T)GetProcAddress(LoadLibraryEx(module_name, nullptr, flags), proc_name);
#define LOAD_LIBRARY_GET_PROC_ADDRESS_ONCE(T, ptr, module_name, flags,         \
                                           proc_name)                          \
    static T ptr;                                                              \
    do {                                                                       \
        static_assert(std::is_trivially_destructible_v<T>);                    \
        static std::once_flag get_proc_address_once_flag_;                     \
        std::call_once(get_proc_address_once_flag_, []() {                     \
            static HMODULE get_proc_address_once_module_ =                     \
                LoadLibraryEx(module_name, nullptr, flags);                    \
            if (get_proc_address_once_module_) {                               \
                ptr = (T)GetProcAddress(get_proc_address_once_module_,         \
                                        proc_name);                            \
                if (!ptr) {                                                    \
                    FreeLibrary(get_proc_address_once_module_);                \
                } else {                                                       \
                    std::atexit(                                               \
                        []() { FreeLibrary(get_proc_address_once_module_); }); \
                }                                                              \
            }                                                                  \
        });                                                                    \
    } while (0)
