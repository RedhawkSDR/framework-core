#ifndef __OMNIJNI_OBJECT_H__
#define __OMNIJNI_OBJECT_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

#include "class.h"

namespace CORBA {

    namespace jni {

        class Object {
        public:
            static void fromJObject (CORBA::Object_var& out, JNIEnv* env, jobject bj);
            static jobject toJObject (CORBA::Object_ptr in, JNIEnv* env);

            static jclass getJClass (JNIEnv* env)
            {
                OnLoad(env);
                return cls_;
            }

            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);
        private:
            static jclass cls_;
            static jmethodID ctor_;
        };


    } // namespace jni

} // namespace CORBA

inline void fromJObject (CORBA::Object_var& out, JNIEnv* env, jobject obj)
{
    CORBA::jni::Object::fromJObject(out, env, obj);
}

inline jobject toJObject (CORBA::Object_ptr in, JNIEnv* env)
{
    return CORBA::jni::Object::toJObject(in, env);
}

template<>
inline jclass getJClass<CORBA::ObjectRef> (JNIEnv* env)
{
    return CORBA::jni::Object::getJClass(env);
}

#endif // __OMNIJNI_OBJECT_H__
