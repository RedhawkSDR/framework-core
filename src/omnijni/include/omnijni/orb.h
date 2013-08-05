#ifndef __OMNIJNI_ORB_H__
#define __OMNIJNI_ORB_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace omnijni {

    class ORB {
    public:
        static void Init (JNIEnv* env);

        static CORBA::Object_ptr object_to_native (JNIEnv* env, jobject obj);

    private:
        static jclass cls_;
        static jmethodID object_to_string_;
    };

};

#endif // __OMNIJNI_ORB_H__
