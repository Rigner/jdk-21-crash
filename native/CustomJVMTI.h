#ifndef BADLIONJVMTI_H
#define BADLIONJVMTI_H

#include "jvmti.h"
#include "jni.h"

class CustomJVMTI
{
public:

    CustomJVMTI(const CustomJVMTI &) = delete;

    CustomJVMTI &operator=(const CustomJVMTI &) = delete;

    CustomJVMTI(CustomJVMTI &&) = delete;

    CustomJVMTI &operator=(CustomJVMTI &&) = delete;

    ~CustomJVMTI() = default;

    static CustomJVMTI &GetInstance()
    {
        static CustomJVMTI instance;
        return instance;
    }

    void VMInit(jint version);

    void Shutdown(JNIEnv *);

    void LoadClass(const std::string &className, jint oldClassDataLen, const unsigned char *oldClassData, jint *newClassDataLen, unsigned char **newClassData, JNIEnv *jni_env, jvmtiEnv *jvmti);

private:
    CustomJVMTI();

    bool usingModernJava; // If using Java 18+

    jclass tweakerClass = nullptr;
};


#endif