#pragma once

#include <string>

/**
 * @class DynamicLibrary
 * @brief Platform-specific dynamic library loader.
 *
 * Encapsulates loading and unloading of a dynamic/shared library
 * at runtime and provides access to exported symbols.
 *
 * @note Current implementation supports Windows only
 *       (LoadLibrary / GetProcAddress).
 */
class DynamicLibrary
{
public:
    /**
     * @brief Loads a dynamic library.
     *
     * Attempts to load the dynamic library specified by name.
     *
     * @param name Name or path of the dynamic library.
     *
     * @note On Windows, this maps to LoadLibraryA().
     * @warning No error checking is performed if loading fails.
     */
    DynamicLibrary(const std::string& name);

    /**
     * @brief Unloads the dynamic library.
     *
     * Frees the loaded library if it was successfully loaded.
     *
     * @note On Windows, this maps to FreeLibrary().
     */
    ~DynamicLibrary();

    /**
     * @brief Retrieves a symbol from the loaded library.
     *
     * @param symbolName Name of the exported symbol.
     *
     * @return Pointer to the symbol, or nullptr if not found.
     *
     * @note On Windows, this maps to GetProcAddress().
     * @warning The returned pointer is untyped and must be cast
     *          to the correct function or object type by the caller.
     */
    void* getSymbol(const std::string& symbolName) const;

    /**
    * @brief Retrieves a symbol from the loaded library with a typed return.
    *
    * Convenience templated overload that casts the symbol pointer
    * to the requested type.
    *
    * @tparam T Desired symbol type (typically a function pointer).
    * @param symbolName Name of the exported symbol.
    *
    * @return Symbol cast to type T, or nullptr if not found.
    *
    * @warning No type safety is enforced. The caller must ensure
    *          that T matches the actual symbol type.
    */
    template<class T>
    T getSymbol(const std::string& symbolName) const
    {
        return (T)getSymbol(symbolName);
    }

private:
    /**
     * @brief Platform-specific library handle.
     *
     * On Windows, this stores an HMODULE cast to void*.
     */
    void* instance;
};
