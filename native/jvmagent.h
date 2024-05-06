#ifndef JVMAGENT_H
#define JVMAGENT_H

#include <jvmti.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* jvm, char* options, void* reserved);

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* jvm);

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* jvm, char *options, void *reserved);

JNIEXPORT void JNICALL Callback_VMInit(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread);

JNIEXPORT void JNICALL Callback_ClassFileLoadHook(jvmtiEnv *jvmti, JNIEnv* jni, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data);

#ifdef __cplusplus
}
#endif


#endif //JVMAGENT_H
