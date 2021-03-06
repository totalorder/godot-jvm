#include <modules/kotlin_jvm/src/wire/wire.pb.h>
#include "kt_property.h"
#include "kt_variant.h"
#include "gd_kotlin.h"

KtPropertyInfo::KtPropertyInfo(jni::JObject p_wrapped, jni::JObject& p_class_loader)
        : JavaInstanceWrapper("godot.core.KtPropertyInfo", p_wrapped, p_class_loader) {
    jni::Env env { jni::Jvm::current_env() };
    jni::MethodId getTypeMethod{get_method_id(env, "getType", "()I")};
    type = KtVariant::fromWireTypeToVariantType(static_cast<wire::Value::TypeCase>(wrapped.call_int_method(env, getTypeMethod)));
    jni::MethodId getNameMethod{get_method_id(env, "getName", "()Ljava/lang/String;")};
    name = env.from_jstring(wrapped.call_object_method(env, getNameMethod));
    jni::MethodId getClassNameMethod{get_method_id(env, "getClassName", "()Ljava/lang/String;")};
    class_name = env.from_jstring(wrapped.call_object_method(env, getClassNameMethod));
    jni::MethodId getPropertyHintMethod{get_method_id(env, "getHint", "()I")};
    hint = static_cast<PropertyHint>(wrapped.call_int_method(env, getPropertyHintMethod));
    jni::MethodId getHintStringMethod{get_method_id(env, "getHintString", "()Ljava/lang/String;")};
    hint_string = env.from_jstring(wrapped.call_object_method(env, getHintStringMethod));
}

PropertyInfo KtPropertyInfo::toPropertyInfo() {
    PropertyInfo info;
    info.type = type;
    info.name = name;
    info.class_name = class_name;
    info.hint = hint;
    info.hint_string = hint_string;
    return info;
}

KtProperty::KtProperty(jni::JObject p_wrapped, jni::JObject& p_class_loader)
        : JavaInstanceWrapper("godot.core.KtProperty", p_wrapped, p_class_loader) {
    jni::Env env { jni::Jvm::current_env() };
    jni::MethodId getKtPropertyInfoMethod{get_method_id(env, "getKtPropertyInfo", "()Lgodot/core/KtPropertyInfo;")};
    propertyInfo = new KtPropertyInfo(wrapped.call_object_method(env, getKtPropertyInfoMethod),
                                      GDKotlin::get_instance().get_class_loader());
}

KtProperty::~KtProperty() {
    delete propertyInfo;
}

StringName KtProperty::get_name() const {
    return propertyInfo->name;
}

PropertyInfo KtProperty::get_member_info() {
    return propertyInfo->toPropertyInfo();
}

Variant KtProperty::callGet(const KtObject* instance) {
    jni::Env env{jni::Jvm::current_env()};
    jni::MethodId getCallMethodId {get_method_id(env, "callGet", "(Lgodot/core/KtObject;)Z")};
    bool refreshBuffer = wrapped.call_boolean_method(env, getCallMethodId, {instance->get_wrapped()});
    return GDKotlin::get_instance().transfer_context->read_return_value(env, refreshBuffer).to_godot_variant();
}

void KtProperty::setCall(const KtObject* instance, const Variant& p_value) {
    jni::Env env{jni::Jvm::current_env()};
    jni::MethodId setCallMethodId {get_method_id(env, "callSet", "(Lgodot/core/KtObject;)V")};
    Vector<KtVariant> arg;
    arg.push_back(KtVariant(p_value));
    GDKotlin::get_instance().transfer_context->write_args(env, arg);
    wrapped.call_void_method(env, setCallMethodId, {instance->get_wrapped()});
}
