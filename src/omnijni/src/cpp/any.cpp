#include <omnijni/any.h>

namespace {
    namespace AnyUtils {
        jclass cls_ = NULL;
        jmethodID from_bytes_ = 0;
        jmethodID to_bytes_ = 0;

        void OnLoad (JNIEnv* env)
        {
            if (cls_) return;

            cls_ = omnijni::loadClass(env, "omnijni.AnyUtils");
            from_bytes_ = env->GetStaticMethodID(cls_, "from_bytes", "([B)Lorg/omg/CORBA/Any;");
            to_bytes_ = env->GetStaticMethodID(cls_, "to_bytes", "(Lorg/omg/CORBA/Any;)[B");
        }

        jobject from_bytes (JNIEnv* env, jbyteArray data)
        {
            return env->CallStaticObjectMethod(cls_, from_bytes_, data);
        }

        jbyteArray to_bytes (JNIEnv* env, jobject any)
        {
            return (jbyteArray)env->CallStaticObjectMethod(cls_, to_bytes_, any);
        }
    }
}

jclass CORBA::jni::Any::cls_ = NULL;

void CORBA::jni::Any::OnLoad (JNIEnv* env)
{
    if (cls_) return;

    AnyUtils::OnLoad(env);
    cls_ = omnijni::loadClass(env, "org.omg.CORBA.Any");
}

void CORBA::jni::Any::fromJObject (CORBA::Any& any, JNIEnv* env, jobject obj)
{
    OnLoad(env);

    if (obj == NULL) {
        any = CORBA::Any();
        return;
    }

    jbyteArray data = AnyUtils::to_bytes(env, obj);
    jint size = env->GetArrayLength(data);
    jbyte* buffer = env->GetByteArrayElements(data, NULL);
    
    cdrMemoryStream stream(buffer, size);
    stream.setByteSwapFlag(false);
    any <<= stream;

    env->ReleaseByteArrayElements(data, buffer, JNI_ABORT);
}

jobject CORBA::jni::Any::toJObject (const CORBA::Any& any, JNIEnv* env)
{
    OnLoad(env);
    cdrMemoryStream output;
    output.setByteSwapFlag(false);
    any >>= output;
    jbyteArray data = env->NewByteArray(output.bufSize());
    env->SetByteArrayRegion(data, 0, output.bufSize(), (jbyte*)output.bufPtr());
    jobject out = AnyUtils::from_bytes(env, data);

    env->DeleteLocalRef(data);

    return out;
}
