#ifndef __OMNIJNI_EXCEPTION_H__
#define __OMNIJNI_EXCEPTION_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace CORBA {

    namespace jni {

        class SystemException
        {
        public:
            static void throwJava (const CORBA::SystemException& ex, JNIEnv* env);
            static void throwNative (JNIEnv* env, jobject obj);
            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);

        private:
            SystemException();
            ~SystemException();
            static jclass cls_;
            static jfieldID minor_;
            static jfieldID completed_;
            static jmethodID get_name_;
        };

    }

}

#endif // __OMNIJNI_EXCEPTION_H__
