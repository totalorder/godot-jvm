package godot.core

object TypeManager {
    private val userTypes = HashSet<String>()
    private val engineTypes = HashSet<String>()

    fun registerUserType(className: String) = userTypes.add(className)

    fun registerEngineType(className: String) = engineTypes.add(className)

    fun isUserType(className: String) = userTypes.contains(className)

    fun isEngineType(className: String) = engineTypes.contains(className)
}
