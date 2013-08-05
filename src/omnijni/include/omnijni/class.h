#ifndef __OMNIJNI_CLASS_H__
#define __OMNIJNI_CLASS_H__

#include <stdexcept>

#include <omniORB4/CORBA.h>
#include <jni.h>

template <class T>
jclass getJClass (JNIEnv* env)
{
    throw std::runtime_error("Missing Java class mapping");
}

namespace omnijni {

    jclass loadClass (JNIEnv* env, const std::string& name);

} // namespace omnijni

#endif // __OMNIJNI_CLASS_H__
