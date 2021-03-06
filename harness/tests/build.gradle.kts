
plugins {
    kotlin("jvm") version "1.4.10"
    id("com.utopia-rise.godot-jvm") version "0.1.0-3.2"
}

repositories {
    jcenter()
}

dependencies {
    implementation("joda-time:joda-time:2.10.6") //external dependency to test dependency inclusion in dummyCompilation
}
