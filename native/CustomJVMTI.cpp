#include <iostream>
#include <string>

#include "CustomJVMTI.h"

#define LOG(message) std::cout << message << std::endl

CustomJVMTI::CustomJVMTI()
        : usingModernJava(false)
{

}

void CustomJVMTI::VMInit(jint version)
{
    this->usingModernJava = version > 0x30110000; // > Java 17
}

void CustomJVMTI::Shutdown(JNIEnv *jni)
{
    if (this->tweakerClass != nullptr)
        jni->DeleteGlobalRef(this->tweakerClass);
}

void CustomJVMTI::LoadClass(const std::string &className, jint oldClassDataLen, const unsigned char *oldClassData, jint *newClassDataLen, unsigned char **newClassData, JNIEnv *jni_env, jvmtiEnv *jvmti)
{
    if (this->tweakerClass == nullptr)
    {
        bool internalClass = false;

#ifdef PREVENT_CRASH
        if (this->usingModernJava)
        {
            internalClass = className.rfind("sun/", 0) == 0
                            || className.rfind("com/sun/", 0) == 0
                            || className.rfind("java/", 0) == 0
                            || className.rfind("javax/", 0) == 0
                            || className.rfind("jdk/", 0) == 0
                            || className == "com/github/psnrigner/jdk21crash/ClassLoadHook";
        }
#endif

        if (!internalClass)
        {
            jclass tweaker = jni_env->FindClass("com/github/psnrigner/jdk21crash/ClassLoadHook");

            if (tweaker != nullptr)
            {
                this->tweakerClass = reinterpret_cast<jclass>(jni_env->NewGlobalRef(tweaker));
            }
            else
            {
                jni_env->ExceptionClear(); // Clear the ClassNotFoundError
                this->tweakerClass = nullptr;
            }

            if (this->tweakerClass == nullptr)
            {
                LOG("[LoadClass] Failed to find ClassLoadHook");
            }
            else
            {
                LOG("[LoadClass] Loaded ClassLoadHook");
            }
        }
    }
}
