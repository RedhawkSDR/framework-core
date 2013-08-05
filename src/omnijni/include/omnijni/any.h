#ifndef __OMNIJNI_ANY_H__
#define __OMNIJNI_ANY_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

#include "class.h"

namespace CORBA {

    namespace jni {

        class Any {
        public:
            static void fromJObject (CORBA::Any& out, JNIEnv* env, jobject obj);
            static jobject toJObject (const CORBA::Any& in, JNIEnv* env);

            static jclass getJClass (JNIEnv* env)
            {
                OnLoad(env);
                return cls_;
            }

            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);
        private:
            static jclass cls_;

        };

    } // namespace jni

} // namespace CORBA

inline void fromJObject(CORBA::Any& out, JNIEnv* env, jobject obj)
{
    CORBA::jni::Any::fromJObject(out, env, obj);
}

inline jobject toJObject(const CORBA::Any& in, JNIEnv* env)
{
    return CORBA::jni::Any::toJObject(in, env);
}

template<>
inline jclass getJClass<CORBA::Any> (JNIEnv* env)
{
    return CORBA::jni::Any::getJClass(env);
}

#endif // __OMNIJNI_ANY_H__
