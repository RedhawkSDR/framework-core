#include <string>
#include <algorithm>
#include <stdexcept>

#include <omnijni/class.h>

jclass omnijni::loadClass (JNIEnv* env, const std::string& name)
{
    std::string desc;
    std::replace_copy(name.begin(), name.end(), std::back_insert_iterator<std::string>(desc), '.', '/');

    jclass cls = env->FindClass(desc.c_str());
    if (cls == NULL) {
        throw std::runtime_error(name + " not found");
    }
    jclass ref = (jclass)env->NewWeakGlobalRef(cls);
    if (ref == NULL) {
        throw std::runtime_error("Failed to create global reference for " + name);
    }
    return ref;
}

