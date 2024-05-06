#include <chrono>
#include <iostream>
#include <string>

#include "jvmagent.h"
#include "CustomJVMTI.h"

#ifndef _WIN32
#include "dllmain.h"
#endif

#define LOG(message) std::cout << message << std::endl

bool CheckJVMTIError(jvmtiEnv *jvmti, jvmtiError error, const char *msg)
{
    if (error == JVMTI_ERROR_NONE)
        return false;

    char *errStr = nullptr;
    jvmti->GetErrorName(error, &errStr);

    if (errStr != nullptr)
        LOG("[CheckJVMTIError] " + std::string(msg) + " -  Error: " + std::string(errStr));
    else
        LOG("[CheckJVMTIError] Unknown Error.");

    return true;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
    // Entry point for JVMTI... Windows uses regular DLL loading hook
#ifndef _WIN32
    InitDLL();
#endif

    jvmtiError error;

    LOG("[Agent_OnLoad] Called!");

    jvmtiEnv *jvmti = nullptr;

    int result = jvm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION);

    if (result != JNI_OK || jvmti == nullptr)
    {
        LOG("[Agent_OnLoad] Unable to get the JVMTI environment: " + std::to_string(result));
        return result;
    }

    LOG("[Agent_OnLoad] Successfully got the JVMTI!");

    jvmtiCapabilities capabilities;
    memset(&capabilities, 0, sizeof(jvmtiCapabilities));

    capabilities.can_generate_all_class_hook_events = 1;
    capabilities.can_redefine_any_class = 1;
    capabilities.can_redefine_classes = 1;
    capabilities.can_retransform_any_class = 1;
    capabilities.can_retransform_classes = 1;

    error = jvmti->AddCapabilities(&capabilities);
    CheckJVMTIError(jvmti, error, "Can't add capabilities.");

    if (error != JVMTI_ERROR_NONE)
        return error;

    LOG("[Agent_OnLoad] Added capabilities.");

    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(jvmtiEventCallbacks));

    callbacks.VMInit = &Callback_VMInit;
    callbacks.ClassFileLoadHook = &Callback_ClassFileLoadHook;

    error = jvmti->SetEventCallbacks(&callbacks, (jint) sizeof(callbacks));
    CheckJVMTIError(jvmti, error, "Can't set callbacks.");

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
    CheckJVMTIError(jvmti, error, "Can't set notification mode for vm_init.");

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    CheckJVMTIError(jvmti, error, "Can't set notification mode for classfileload.");

    LOG("[Agent_OnLoad] Done!");

    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *jvm)
{
    JNIEnv *env;
    JavaVMAttachArgs vmAttachArgs;
    vmAttachArgs.name = const_cast<char *>("jdk-21-crash-agent");
    vmAttachArgs.version = JNI_VERSION_1_8;
    vmAttachArgs.group = nullptr;
    jvm->AttachCurrentThread((void **) &env, &vmAttachArgs);

    CustomJVMTI::GetInstance().Shutdown(env);

    jvm->DetachCurrentThread();

    LOG("[OnUnload] Called!");
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options, void *reserved)
{
    LOG("[OnAttach] Called!");

    return JNI_TRUE;
}

JNIEXPORT void JNICALL Callback_VMInit(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread)
{
    LOG("[VMInit] Called.");

    jint version;
    jvmtiError error = jvmti->GetVersionNumber(&version);
    CheckJVMTIError(jvmti, error, "Unable to grab the VM's version.");

    if (error == JVMTI_ERROR_NONE)
        LOG("[VMInit] VM's version is: " + std::to_string(version));

    CustomJVMTI::GetInstance().VMInit(version);
}

JNIEXPORT void JNICALL Callback_ClassFileLoadHook(jvmtiEnv *jvmti, JNIEnv *jni, jclass class_being_redefined, jobject loader, const char *name, jobject protection_domain, jint class_data_len, const unsigned char *class_data, jint *new_class_data_len, unsigned char **new_class_data)
{
    if (jni == nullptr) // Sometimes it's null on Java 8 ?
        return;

    LOG("[ClassFileLoadHook] Called.");
    if (name == nullptr)
    {
        LOG("[ClassFileLoadHook] No class name");
        return;
    }

    LOG("[ClassFileLoadHook] Loaded class: " + std::string(name) + " with loader " + std::to_string(loader == nullptr));

    jvmtiPhase phase;

    jvmti->GetPhase(&phase);

    LOG("[ClassFileLoadHook] Phase: " + std::to_string((int) phase));

    if (phase != JVMTI_PHASE_LIVE && phase != JVMTI_PHASE_START && phase != JVMTI_PHASE_PRIMORDIAL)
    {
        LOG("[ClassFileLoadHook] Incorrect phase.");
        return;
    }

    CustomJVMTI::GetInstance().LoadClass(name, class_data_len, class_data, new_class_data_len, new_class_data, jni, jvmti);
}
