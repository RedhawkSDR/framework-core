#include <jni.h>

#include <omnijni/orb.h>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM* jvm, void* reserved)
{
    // Map to the JVM enviroment.
    JNIEnv* env;
    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_2)) {
        return false;
    }

    // Must initialize the CORBA ORB before anything else happens.
    omnijni::ORB::Init(env);

    return JNI_VERSION_1_2;
}
