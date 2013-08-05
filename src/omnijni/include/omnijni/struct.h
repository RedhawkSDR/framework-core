#ifndef __OMNIJNI_STRUCT_H__
#define __OMNIJNI_STRUCT_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace omnijni {

    inline void fromJObject (_CORBA_String_member& str, JNIEnv* env, jobject obj)
    {
        jstring jstr = (jstring)obj;
        str = env->GetStringUTFChars(jstr, NULL);
        env->ReleaseStringUTFChars(jstr, NULL);
    }

    inline void fromJObject (CORBA::Object_Member& member, JNIEnv* env, jobject obj)
    {
        // TODO
        member = CORBA::Object::_nil();
    }

    template <class T, class T_Helper>
    void fromJObject(_CORBA_ObjRef_Member<T,T_Helper>& member, JNIEnv* env, jobject obj)
    {
        // TODO
    }

    template <class T>
    void getObjectField (T& out, JNIEnv* env, jobject obj, jfieldID fid) {
        jobject field = env->GetObjectField(obj, fid);
        fromJObject(out, env, field);
    }

}

#endif // __OMNIJNI_STRUCT_H__
