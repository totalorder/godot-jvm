#include "transfer_context.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "gd_kotlin.h"
#include "kotlin_instance.h"

TransferContext::TransferContext(jni::JObject p_wrapped, jni::JObject p_class_loader)
    : JavaInstanceWrapper("godot.core.TransferContext", p_wrapped, p_class_loader) {
    jni::JNativeMethod icall_method {
        "icall",
        "(JLjava/lang/String;Ljava/lang/String;IZ)V",
        (void*) TransferContext::icall
    };

    jni::JNativeMethod invoke_ctor_method {
        "invokeConstructor",
        "(Ljava/lang/String;)J",
        (void*) TransferContext::invoke_constructor
    };

    jni::JNativeMethod set_script_method {
            "setScript",
            "(JLjava/lang/String;Lgodot/core/KtObject;Ljava/lang/ClassLoader;)V",
            (void*) TransferContext::set_script
    };

    jni::JNativeMethod free_object_method {
            "freeObject",
            "(J)V",
            (void*) TransferContext::free_object
    };

    Vector<jni::JNativeMethod> methods;
    methods.push_back(icall_method);
    methods.push_back(invoke_ctor_method);
    methods.push_back(set_script_method);
    methods.push_back(free_object_method);
    jni::Env env {jni::Jvm::current_env()};
    get_class(env).register_natives(env, methods);
}

TransferContext::SharedBuffer* TransferContext::get_buffer(jni::Env& p_env, bool p_refresh_buffer) {
    thread_local static TransferContext::SharedBuffer shared_buffer;

    if (!shared_buffer.ptr || p_refresh_buffer) {

        jni::MethodId method = get_method_id(p_env, "getBuffer", "()Ljava/nio/ByteBuffer;");
        jni::JObject buffer = wrapped.call_object_method(p_env, method);
        assert(!buffer.isNull());
        shared_buffer = SharedBuffer {
                p_env.get_direct_buffer_address(buffer),
                p_env.get_direct_buffer_capacity(buffer)
        };
    }

    return &shared_buffer;
}

bool TransferContext::ensure_capacity(jni::Env& p_env, long p_capacity) {
    jni::MethodId method = get_method_id(p_env, "ensureCapacity", "(I)Z");
    return wrapped.call_boolean_method(p_env, method, {static_cast<jlong>(p_capacity)});
}

void TransferContext::write_return_value(jni::Env& p_env, const KtVariant& p_value) {
    wire::ReturnValue ret_val;
    ret_val.mutable_data()->CopyFrom(p_value.get_value());

    SharedBuffer* buffer { get_buffer(p_env, ensure_capacity(p_env, ret_val.ByteSizeLong())) };
    google::protobuf::io::ArrayOutputStream os(buffer->ptr, buffer->capacity);
    google::protobuf::io::CodedOutputStream cos(&os);
    google::protobuf::util::SerializeDelimitedToCodedStream(ret_val, &cos);
}

KtVariant TransferContext::read_return_value(jni::Env& p_env, bool p_refresh_buffer) {
    wire::ReturnValue ret_val;

    SharedBuffer* buffer { get_buffer(p_env, p_refresh_buffer) };
    google::protobuf::io::ArrayInputStream is(buffer->ptr, buffer->capacity);
    google::protobuf::io::CodedInputStream cis(&is);
    bool cleanEof;
    google::protobuf::util::ParseDelimitedFromCodedStream(&ret_val, &cis, &cleanEof);
    return KtVariant(ret_val.data());
}

void TransferContext::write_args(jni::Env& p_env, const Vector<KtVariant>& p_args) {
    wire::FuncArgs func_args;
    for (auto i = 0; i < p_args.size(); i++) {
        func_args.add_args()->CopyFrom(p_args.get(i).get_value());
    }

    SharedBuffer* buffer { get_buffer(p_env, ensure_capacity(p_env, func_args.ByteSizeLong())) };
    google::protobuf::io::ArrayOutputStream os(buffer->ptr, buffer->capacity);
    google::protobuf::io::CodedOutputStream cos(&os);
    google::protobuf::util::SerializeDelimitedToCodedStream(func_args, &cos);
}

Vector<KtVariant> TransferContext::read_args(jni::Env& p_env, bool p_refresh_buffer) {
    wire::FuncArgs k_args;

    SharedBuffer* buffer { get_buffer(p_env, p_refresh_buffer) };
    google::protobuf::io::ArrayInputStream is(buffer->ptr, buffer->capacity);
    google::protobuf::io::CodedInputStream cis(&is);
    bool cleanEof;
    google::protobuf::util::ParseDelimitedFromCodedStream(&k_args, &cis, &cleanEof);
    Vector<KtVariant> args;
    for (const auto& k_arg : k_args.args()) {
        args.push_back(KtVariant(k_arg));
    }
    return args;
}

void TransferContext::icall(JNIEnv* rawEnv, jobject instance, jlong jPtr,
           jstring jClassName,
           jstring jMethod, jint expectedReturnType,
           bool p_refresh_buffer) {
    thread_local static Variant variantArgs[MAX_ARGS_SIZE];
    thread_local static const Variant* variantArgsPtr[MAX_ARGS_SIZE];
    thread_local static bool icall_args_init = false;
    if (!icall_args_init) {
        for (int i = 0; i < MAX_ARGS_SIZE; i++) {
            variantArgsPtr[i] = &variantArgs[i];
        }
        icall_args_init = true;
    }

    TransferContext* transferContext{GDKotlin::get_instance().transfer_context};
    const jni::JObject& classLoader = GDKotlin::get_instance().get_class_loader();
    jni::Env env(rawEnv);
    Vector<KtVariant> tArgs = transferContext->read_args(env, p_refresh_buffer);
    int argsSize = tArgs.size();

    ERR_FAIL_COND_MSG(argsSize > MAX_ARGS_SIZE, vformat("Cannot have more than %s arguments for method call.", MAX_ARGS_SIZE))

    for (int i = 0; i < argsSize; i++) {
        variantArgs[i] = tArgs[i].to_godot_variant();
    }

    auto* ptr = reinterpret_cast<Object*>(jPtr);
    String className = env.from_jstring(jni::JString(jClassName));
    String method = env.from_jstring(jni::JString(jMethod));

    Variant::CallError r_error{Variant::CallError::CALL_OK};
    MethodBind* methodBind{ClassDB::get_method(className, method)};
    ERR_FAIL_COND_MSG(!methodBind, vformat("Cannot find method %s in class %s", method, className))
    const KtVariant& retValue{methodBind->call(ptr, variantArgsPtr, argsSize, r_error)};
    ERR_FAIL_COND_MSG(r_error.error != Variant::CallError::CALL_OK, vformat("Call to %s failed.", method))
    transferContext->write_return_value(env, retValue);
}

jlong TransferContext::invoke_constructor(JNIEnv *p_raw_env, jobject p_instance, jstring p_class_name) {
    jni::Env env(p_raw_env);
    StringName class_name = env.from_jstring(jni::JString(p_class_name));
    Object* ptr = ClassDB::instance(class_name);
    ERR_FAIL_COND_V_MSG(!ptr, 0, vformat("Failed to instantiate class %s", class_name))
    return reinterpret_cast<uintptr_t>(ptr);
}

void TransferContext::set_script(JNIEnv *p_raw_env, jobject p_instance, jlong p_raw_ptr, jstring p_class_name,
                                 jobject p_object, jobject p_class_loader) {
    jni::Env env(p_raw_env);
    StringName class_name = env.from_jstring(jni::JString(p_class_name));
    auto* owner = reinterpret_cast<Object*>(p_raw_ptr);
    auto* kt_object = new KtObject(jni::JObject(p_object), jni::JObject(p_class_loader), class_name);
    auto* script = memnew(KotlinInstance(kt_object, owner, GDKotlin::get_instance().find_class_by_name(class_name)));
    owner->set_script_instance(script);
}

void TransferContext::free_object(JNIEnv *p_raw_env, jobject p_instance, jlong p_raw_ptr) {
    auto* owner = reinterpret_cast<Object*>(static_cast<uintptr_t>(p_raw_ptr));
    owner->call_multilevel("free");
}
