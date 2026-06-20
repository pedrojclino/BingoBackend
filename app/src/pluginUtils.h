#pragma once

#include <dlfcn.h>

#include <string>

template<typename T>
class Plugin
{
public:
    typedef T* (*CreateObjectFunc)();
    typedef void (*DestroyObjectFunc)(T*);

    ~Plugin() 
    {
        // Cleanup (Must use the library's destroy function)
        if (m_plugin)
            m_destroy_plugin(m_plugin);

        if (m_handle)
            dlclose(m_handle);
    }

    bool loadLibrary(const std::string_view& pluginName, std::string& errors)
    {
        // Open the library
        m_handle = dlopen(pluginName.data(), RTLD_LAZY);
        if (!m_handle)
        {
            errors += "dlopen failed: " + std::string(dlerror()) + "\n";
            return false;
        }

        // Load the factory symbols
        CreateObjectFunc create_plugin = (CreateObjectFunc)dlsym(m_handle, "createPlugin");
        m_destroy_plugin = (DestroyObjectFunc)dlsym(m_handle, "destroyPlugin");

        if (!create_plugin || !m_destroy_plugin)
        {
            errors += "dlsym failed: " + std::string(dlerror()) + "\n";
            dlclose(m_handle);
            return false;
        }

        // Use the plugin via the Base Class pointer
        m_plugin = create_plugin();
        return true;
    }

    T* plugin() { return m_plugin; };

private:

    T* m_plugin{ nullptr };

    void* m_handle{ nullptr };
    DestroyObjectFunc m_destroy_plugin{ nullptr };
};