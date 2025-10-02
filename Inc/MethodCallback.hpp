#ifndef METHODCALLBACK_HPP
#define METHODCALLBACK_HPP

// Classe générique qui encapsule un pointeur vers méthode d'instance
template <typename T>
class MethodCallback {
public:
    typedef void (T::*Method)();

    MethodCallback(T* instance = 0, Method method = 0)
        : m_instance(instance), m_method(method) {}

    void operator()() {
        if (m_instance && m_method) {
            (m_instance->*m_method)();
        }
    }

    bool isValid() const {
        return (m_instance != 0 && m_method != 0);
    }

private:
    T* m_instance;
    Method m_method;
};

#endif // METHODCALLBACK_HPP
