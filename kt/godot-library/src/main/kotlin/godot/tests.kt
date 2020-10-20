package godot

import godot.core.*
import godot.util.VoidPtr

open class Object : KtObject() {
    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("Object")
    }

    fun getInstanceId(): Long {
        TransferContext.writeArguments()
        TransferContext.callMethod(rawPtr, "Object", "get_instance_id", KtVariant.Type.LONG, false)
        return TransferContext.readReturnValue().asLong()
    }
}

open class Node : Object() {
    open var name: String
        get() {
            TransferContext.writeArguments()
            TransferContext.callMethod(rawPtr, "Node", "get_name", KtVariant.Type.STRING, false)
            return TransferContext.readReturnValue().asString()
        }
        set(value) {
            val refresh = TransferContext.writeArguments(KtVariant(value))
            TransferContext.callMethod(rawPtr, "Node", "set_name", KtVariant.Type.NIL, refresh)
            TransferContext.readReturnValue()
        }

    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("Node")
    }

    open fun getParent(): Node {
        val refresh = TransferContext.writeArguments()
        TransferContext.callMethod(rawPtr, "Node", "get_parent", KtVariant.Type.OBJECT, refresh)
        return TransferContext.readReturnValue().asObject()
    }
}

open class Spatial : Node() {
    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("Spatial")
    }
}

open class Reference : Object(), KtReference {
    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("Reference")
    }

    override val referencePtr: VoidPtr
        get() = rawPtr
}

open class Resource : Reference() {
    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("Resource")
    }
}

open class NavigationMesh : Resource() {
    override fun __new(): VoidPtr {
        return TransferContext.invokeConstructor("NavigationMesh")
    }
}

fun registerEngineTypes() {
    TypeManager.registerEngineType("Object", ::Object)
    TypeManager.registerEngineType("Node", ::Node)
    TypeManager.registerEngineType("Spatial", ::Spatial)
    TypeManager.registerEngineType("Reference", ::Reference)
    TypeManager.registerEngineType("Resource", ::Resource)
    TypeManager.registerEngineType("NavigationMesh", ::NavigationMesh)
}
